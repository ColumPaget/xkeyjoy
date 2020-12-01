#include "common.h"
#include "X11.h"
#include "profile.h"
#include "evdev.h"
#include "wait.h"

char *ConfigPath=NULL;

//starts off as true, gets set false after reload, can be
//set to true again by SIGHUP
int ProfilesNeedReload=TRUE;

void SignalHandler(int sig)
{
	if (sig==SIGHUP) ProfilesNeedReload=TRUE;
}


int IfEventTrigger(TInputMap *ev, TInputMap *IMap)
{
//sendkey actions trigger on both up and down
if (IMap->action==ACT_SENDKEY) return(TRUE);

//other actions, like 'exec' only trigger on release or
//on specified values
switch (ev->intype)
{
	case EV_SW: 
		if (ev->value==IMap->value) return(TRUE);
	break;

	default:
		//for buttons trigger on button release
		if (ev->value==0) return(TRUE);
	break;
}

return(FALSE);
}


int ProcessEvent(TProfile *Profile, Window win, TInputMap *ev)
{
TInputMap *IMap;
int i, active=FALSE, matched=FALSE;
Window target;
pid_t pid;

if (! Profile) return(FALSE);
for (i=0; i < Profile->NoOfEvents; i++)
{
	IMap=(TInputMap *) &Profile->Events[i];	
	target=X11FindWin(IMap->target);
	if (! target) target=win;

	if ((ev->intype==EV_XKB) && (IMap->intype==EV_XKB))
	{
					//printf("XKB: value=%d %d:%d %d:%d\n", ev->value, ev->input, IMap->input, ev->inmods, IMap->inmods);
	}
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
					X11SendEvent(target, IMap->output, IMap->outmods, TRUE);
					IMap->flags |= FLAG_ACTIVE;
				}
				else if (IMap->flags & FLAG_ACTIVE) 
				{
					X11SendEvent(target, IMap->output, IMap->outmods, FALSE);
					IMap->flags &= ~FLAG_ACTIVE;
				}

			break;


			case EV_SW:
			case EV_XKB:
				//fall through
				
			default:
						if (IfEventTrigger(ev, IMap))
						{
							switch(IMap->action)
							{
								case ACT_EXEC:
								printf("RUN COMMAND: %s\n", IMap->target + 5);
								Spawn(IMap->target +5, "");
								break;

								case ACT_WINCLOSE:
								case ACT_WINKILL:
								X11CloseWindow(target, IMap->action);
								break;

								case ACT_WINSHADE:
								case ACT_WINHIDE:
								case ACT_WINSTICK:
								case ACT_WINFULLSCR:
								case ACT_WINMAX_X:
								case ACT_WINMAX_Y:
								case ACT_WINRAISED:
								case ACT_WINLOWERED:
								X11WindowSetState(target, IMap->action);
								break;

								default:
									X11SendEvent(target, IMap->output, IMap->outmods, ev->value);
								break;
							}
						}
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

//if the event is of type EV_ABS it means it's a joystick or other type of multi-value axis
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

//We do not do anything regarding to keygrabs here. This is because we
//don't change keygrabs every time a window changes, instead we listen to
//all keygrabs for all windows all the time, and then decide what to do when
//a key is pressed. If the right window for a grab is not currently active
//then we just send the keystroke to the current window
//Thus grabs are only registered when we ReloadProfiles
Tempstr=X11WindowGetCmdLine(Tempstr, win);
Profile=ProfileForApp(Tempstr);
if (Flags & FLAG_DEBUG) printf("Winchange: %s %x\n", Tempstr, win);

Destroy(Tempstr);

return(Profile);
}


void ReloadProfiles(Window FocusWin)
{
TProfile *Grabs;

	Grabs=ProfilesReload(ConfigPath);
	ProfilesNeedReload=FALSE;

	//we only setup key/button grabs with X11 when we load our config 
	//ProfilesReload returns a list of all grabs in the config file
	//and we listen to all of them, and then decide whether the current
	//window has a grab registered when a grab happens. If it doesn't
	//we just pass the keystroke through to the current window
	X11SetupGrabs(Grabs);
}



void HandleX11Keygrabs(Window win, TProfile *Profile)
{
TInputMap ev;

X11GetEvent(&ev);
if (! ProcessEvent(Profile, win, &ev)) X11SendEvent(win, ev.input, ev.inmods, ev.value);
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
else if (strcmp(p_arg, "-v")==0) Flags |= FLAG_VERSION;
else if (strcmp(p_arg, "-version")==0) Flags |= FLAG_VERSION;
else if (strcmp(p_arg, "--version")==0) Flags |= FLAG_VERSION;
else if (strcmp(p_arg, "-h")==0) Flags |= FLAG_HELP;
else if (strcmp(p_arg, "-?")==0) Flags |= FLAG_HELP;
else if (strcmp(p_arg, "-help")==0) Flags |= FLAG_HELP;
else if (strcmp(p_arg, "--help")==0) Flags |= FLAG_HELP;
else if (strcmp(p_arg, "-c")==0) ConfigPath=CopyStr(ConfigPath, CommandLineNext(CmdLine));
p_arg=CommandLineNext(CmdLine);

}

return(Flags);
}


void DisplayHelp()
{
printf("Usage:\n");
printf("   xkeyjoy [options]\n");
printf("   xkeyjoy list\n");
printf("   xkeyjoy mon <device name>\n");
printf("\n");
printf("'xkeyjoy' without a modifer (list, mon) will run in daemon mode and process any devices that either have switch, or axis inputs (lid switch, rfkill, gamepads, joysticks etc).\n");
printf("xkeyjoy list' lists input devices that xkeyjoy can currently receive events from.\n");
printf("'xkeyjoy mon' can be used to monitor events coming from a specific device. e.g. 'xkeyjoy mon event1'\n");
printf("\nOptions for daemon mode are:\n");
printf("-d          don't background/daemonize\n");
printf("-c <path>   path to config file or directory containing config files\n");
printf("-v          output version info\n");
printf("-version    output version info\n");
printf("--version   output version info\n");
printf("-h          this help\n");
printf("-?          this help\n");
printf("-help       this help\n");
printf("--help      this help\n");
}



void ActivateInputs(ListNode *Inputs, ListNode *Devices, STREAM *X11Input)
{
ListNode *Curr;
TEvDev *Dev;

	ListClear(Inputs, NULL);
	ListAddItem(Inputs, X11Input);

	Curr=ListGetNext(Devices);
	while (Curr)
	{
		Dev=(TEvDev *) Curr->Item;
		if (Dev->S != NULL) 
		{
			if (
					BitIsSet(& (Dev->caps), EV_ABS) || 
					BitIsSet(& (Dev->caps), EV_SW)
				 ) 
			{
				ListAddItem(Inputs, Dev->S);
			}
		}
		Curr=ListGetNext(Curr);
	}
}


void main(int argc, char *argv[])
{
ListNode *Devices, *Curr;
ListNode *Inputs;
Window win, prev_win=None;
STREAM *S, *X11Input;
char *Tempstr=NULL;
TProfile *Profile=NULL;
struct input_event ev;
int result, i;
struct timeval tv;

Devices=ListCreate();
ConfigPath=CopyStr(ConfigPath, "/etc/xkeyjoy:~/.xkeyjoy");
signal(SIGHUP, SignalHandler);

Flags=ParseCommandLine(argc, argv);
if ( (Flags & FLAG_NODEMON) && (isatty(1)) ) Flags |= FLAG_DEBUG;

EvdevLoadDevices(Devices);

if (Flags & FLAG_HELP) DisplayHelp();
else if (Flags & FLAG_VERSION) printf("version: %s\n", VERSION);
else if (Flags & FLAG_LISTDEVS) EvdevListDevices(Devices);
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
	ActivateInputs(Inputs, Devices, X11Input);

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

		tv.tv_sec=1;
		tv.tv_usec=0;
		S=STREAMSelect(Inputs, &tv);
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
					if (Flags & FLAG_DEBUG) printf("REMOVE %s\n", S->Path);
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
		
		if (EvdevLoadDevices(Devices)) ActivateInputs(Inputs, Devices, X11Input);
	}
}

Destroy(Tempstr);
}

