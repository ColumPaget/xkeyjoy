#include "common.h"
#include "X11.h"
#include "profile.h"
#include "evdev.h"
#include "wait.h"

#define FLAG_LISTDEVS 1
#define FLAG_MONITOR  2
#define FLAG_NODEMON  4

char *ConfigPath=NULL;

//starts off as true, gets set false after reload, can be
//set to true again by SIGHUP
int ProfilesNeedReload=TRUE;

void SignalHandler(int sig)
{
	if (sig==SIGHUP) ProfilesNeedReload=TRUE;
}




int ProcessEvent(TProfile *Profile, Window win, TInputMap *ev)
{
TInputMap *IMap;
int i, active=FALSE, matched=FALSE;
Window target;

if (! Profile) return(FALSE);
for (i=0; i < Profile->NoOfEvents; i++)
{
	IMap=(TInputMap *) &Profile->Events[i];	
	target=X11FindWin(IMap->target);
	if (! target) target=win;

	if ( (ev->intype==IMap->intype) && (ev->input==IMap->input) && (ev->inmods==IMap->inmods) )
	{
		matched=TRUE;
		active=FALSE;
		switch (ev->intype)
		{
			case EV_ABS:
				if (IMap->flags & ABS_MORE)  
				{
					if (ev->value > IMap->value) active=TRUE;
				}
				else 
				{
					if (ev->value < IMap->value) active=TRUE;
				}

				//we only want to send a key release if Ax->flags has AXIS_ACTIVE set. 
				//Don't send keyreleases if key isn't down
				
				if (active) 
				{
					X11SendKey(target, IMap->output, IMap->outmods, TRUE);
					IMap->flags |= FLAG_ACTIVE;
				}
				else if (IMap->flags & FLAG_ACTIVE) 
				{
					X11SendKey(target, IMap->output, IMap->outmods, FALSE);
					IMap->flags &= ~FLAG_ACTIVE;
				}

			break;

			case EV_XKB:
			default:
					if (IMap->output==0)
					{
						if ( (ev->value == 0) && (strncmp(IMap->target, "exec:", 5)==0) ) Spawn(IMap->target +5, "");
					}
					else X11SendKey(target, IMap->output, IMap->outmods, ev->value);
			break;
		}
	}
}

return(matched);
}


int ProcessDevice(STREAM *S, Window win, struct input_event *ev, TProfile *Profile)
{
TEvDev *Dev;
TAxis *Ax=NULL;
float perc;
int val;
TInputMap IMap;

memset(&IMap, 0, sizeof(IMap));
IMap.intype=ev->type;
IMap.input=ev->code;
IMap.value=ev->value;

if (ev->type==EV_ABS)
{
	switch (ev->code)
	{
		//all the evidence I've seen, including devices I own, implies that these
		//'HAT' axes can only range from -1 to 1
		case ABS_HAT0X:
		case ABS_HAT0Y:
				IMap.value=ev->value * 32767;
		break;

		default:
			Dev=(TEvDev *) STREAMGetItem(S, "evdev");
			if (Dev)
			{
					Ax=&Dev->Axis[ev->code];
					perc=( ((float) (ev->value - Ax->min)) / ((float) (Ax->max - Ax->min)));
					IMap.value = (perc * 32767 * 2) - 32768;
			}
		break;
	}
}

ProcessEvent(Profile, win, &IMap);
}



TProfile *HandleWindowChange(Window win)
{
char *Tempstr=NULL;
TProfile *Profile;

//X11ReleaseKeygrabs(win);
Tempstr=X11WindowGetCmdLine(Tempstr, win);

Profile=ProfileForApp(Tempstr);
printf("Winchange: %s %d\n", Tempstr, Profile);

Destroy(Tempstr);

return(Profile);
}


void ReloadProfiles(Window FocusWin)
{
TProfile *Grabs;

	Grabs=ProfilesReload(ConfigPath);
	ProfilesNeedReload=FALSE;

	X11SetupGrabs(Grabs);
}



void HandleX11Keygrabs(Window win, TProfile *Profile)
{
TInputMap ev;

X11ProcessEvents(&ev);
if (! ProcessEvent(Profile, win, &ev)) X11SendKey(win, ev.input, ev.inmods, ev.value);
}


int ParseCommandLine(int argc, char *argv[])
{
CMDLINE *CmdLine;
const char *p_arg;
int Flags=0;

CmdLine=CommandLineParserCreate(argc, argv);

p_arg=CommandLineFirst(CmdLine);
if (p_arg)
{
	if (strcmp(p_arg, "list")==0) 
	{
		Flags |= FLAG_LISTDEVS;
		p_arg=CommandLineNext(CmdLine);
	}
	else if (strcmp(p_arg, "mon")==0) 
	{
		Flags |= FLAG_MONITOR;
		p_arg=CommandLineFirst(CmdLine);
		if (! StrValid(p_arg))
		{
			printf("ERROR: no device-name given for monitor. Cannot continue\n");
			exit(1);
		}	
		p_arg=CommandLineFirst(CmdLine);
	}
}

while (p_arg)
{
if (strcmp(p_arg, "-d")==0) Flags |= FLAG_NODEMON;
if (strcmp(p_arg, "-c")==0) ConfigPath=CopyStr(ConfigPath, CommandLineNext(CmdLine));
p_arg=CommandLineNext(CmdLine);

}

return(Flags);
}


void main(int argc, char *argv[])
{
ListNode *Devices, *Curr;
ListNode *Inputs;
Window win, prev_win=None;
STREAM *S, *X11Input;
TEvDev *Dev;
char *Tempstr=NULL;
TProfile *Profile=NULL;
struct input_event ev;
int result, Flags=0, i;


ConfigPath=CopyStr(ConfigPath, "/etc/xkeyjoy:~/.xkeyjoy");
signal(SIGHUP, SignalHandler);

Flags=ParseCommandLine(argc, argv);

Devices=EvdevLoadDevices();

if (Flags & FLAG_LISTDEVS) EvdevListDevices(Devices);
else if (Flags & FLAG_MONITOR) EvdevMonitorDevice(Devices, argv[2]);
else
{
	Inputs=ListCreate();
	result=X11Init();
	if (result==-1)
	{
 	 printf("XOpenDisplay() failed. Cannot continue.\n");
		exit(1);
	}

	X11Input=STREAMFromFD(result);
	ListAddItem(Inputs, X11Input);

	Curr=ListGetNext(Devices);
	while (Curr)
	{
		Dev=(TEvDev *) Curr->Item;
		if (Dev->S != NULL) 
		{
			if (
					BitIsSet(& (Dev->caps), EV_ABS)
				 ) 
			{
				ListAddItem(Inputs, Dev->S);
			}
		}
		Curr=ListGetNext(Curr);
	}

	if (! (Flags & FLAG_NODEMON)) demonize();

	while (1)
	{
		if (ProfilesNeedReload)
		{							
			win=X11GetFocusedWin();
			ReloadProfiles(win);
			Profile=HandleWindowChange(win);
			prev_win=win;
		}

		S=STREAMSelect(Inputs, NULL);
		if (S)
		{
				win=X11GetFocusedWin();
				if (win != prev_win) Profile=HandleWindowChange(win);
				prev_win=win;

			if (S==X11Input) HandleX11Keygrabs(win, Profile);
			else
			{
				result=STREAMReadBytes(S, (char *) &ev, sizeof(struct input_event));
				if ((result > 0) && (ev.type != EV_SYN))
				{
				if (Profile) ProcessDevice(S, win, &ev, Profile);
				}
				else if (result < 1)
				{
					ListDeleteItem(Inputs, S);
					STREAMClose(S);
				}
			}
		}

		//collect exited child processes
		for (i=0; i < 100; i++) 
		{
			if (waitpid(-1, NULL, WNOHANG)==-1) break;
		}
	}
}

Destroy(Tempstr);
}

