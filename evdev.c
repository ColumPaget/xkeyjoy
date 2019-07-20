#include "evdev.h"
#include "profile.h"
#include <linux/input.h>

void EvdevPrintDevice(TEvDev *Dev, const char *EventID)
{
  int axisindex;
  uint8_t abs_bitmask[ABS_MAX/8 + 1];
  float percent_deadzone;
  struct input_absinfo abs_features;
	STREAM *S;

	S=Dev->S;
	if (! S) printf("%s %s ERROR: cannot open device\n", EventID, Dev->name);
	else
	{
  memset(abs_bitmask, 0, sizeof(abs_bitmask));
  if (ioctl(S->in_fd, EVIOCGBIT(EV_ABS, sizeof(abs_bitmask)), abs_bitmask) < 0)
    perror("evdev ioctl");

  printf("%s %s ", EventID, Dev->name);
  if (BitIsSet(&(Dev->caps), EV_ABS)) printf(" abs ");
  if (BitIsSet(&(Dev->caps), EV_KEY)) printf(" key ");
  if (BitIsSet(&(Dev->caps), EV_FF)) printf(" ff ");
  if (BitIsSet(&(Dev->caps), EV_LED)) printf(" led ");
  printf("\n");

  for(axisindex = 0; axisindex < ABS_MAX; ++axisindex)
  {
    if(BitIsSet(abs_bitmask, axisindex))
    {
      // This means that the bit is set in the axes list
      printf("  Absolute axis 0x%02x (%d)", axisindex, axisindex);
      //printAxisType(axisindex);

      if(ioctl(S->in_fd, EVIOCGABS(axisindex), &abs_features))
        perror("evdev EVIOCGABS ioctl");

      percent_deadzone = (float)abs_features.flat * 100 / (float)abs_features.maximum;
      printf("(min: %d, max: %d, flatness: %d (=%.2f%%), fuzz: %d)\n",
        abs_features.minimum, abs_features.maximum, abs_features.flat,
        percent_deadzone, abs_features.fuzz);
    }
  }
	}
}


void EvdevListDevices(ListNode *Devices)
{
ListNode *Curr;
TEvDev *Dev;

  Curr=ListGetNext(Devices);
  while (Curr)
  {
  Dev=(TEvDev *) Curr->Item;
	EvdevPrintDevice(Dev, Curr->Tag);
  Curr=ListGetNext(Curr);
  }
}


void EvdevMonitorDevice(ListNode *Devices, const char *Name)
{
ListNode *Node;
struct input_event ev;
char *Tempstr=NULL;
STREAM *StdOut;
TEvDev *Dev;
int Axis[10];
int i, result;

Node=ListFindNamedItem(Devices, Name);
if (Node)
{
Dev=(TEvDev *) Node->Item;
memset(Axis, 0, sizeof(Axis));
StdOut=STREAMFromFD(1);
TerminalClear(StdOut);
TerminalCursorMove(StdOut, 0, 0);
Tempstr=MCopyStr(Tempstr, Name, ":  ", Dev->name, "~>", NULL);
TerminalPutStr(Tempstr, StdOut);

while (1)
{
	while (STREAMCheckForBytes(Dev->S))
	{
	result=STREAMReadBytes(Dev->S, (char *) &ev, sizeof(struct input_event));
	if (ev.type==EV_ABS) Axis[ev.code]=ev.value;
	}

	for (i=0; i < 6; i++)
	{
		TerminalCursorMove(StdOut, 0, i+2);
		Tempstr=FormatStr(Tempstr, "axis: %d  %d~>", i, Axis[i]);
		TerminalPutStr(Tempstr, StdOut);
	}
	STREAMFlush(StdOut);
	usleep(20000);
}
}

Destroy(Tempstr);
}



void EvdevLoadAxes(TEvDev *Ev)
{
struct input_absinfo abs_features;
float percent_deadzone=0;
int i;

//printf("%s\n", Ev->name);
//for(i = 0; i < ABS_MAX; i++)
for(i = 0; i < 6; i++)
{
  //if(BitIsSet(abs_bitmask, axisindex))
  {
    // This means that the bit is set in the axes list

    if(ioctl(Ev->S->in_fd, EVIOCGABS(i), &abs_features) ==0) 
		{
		Ev->Axis[i].min=abs_features.minimum;
		Ev->Axis[i].max=abs_features.maximum;

    // percent_deadzone = (float)abs_features.flat * 100 / (float)abs_features.maximum;
    // printf("    (min: %d, max: %d, flatness: %d (=%.2f%%), fuzz: %d)\n", abs_features.minimum, abs_features.maximum, abs_features.flat, percent_deadzone, abs_features.fuzz);
		}

  }
}
}



ListNode *EvdevLoadDevices()
{
char *Tempstr=NULL, *Path=NULL;
ListNode *Devices;
glob_t Glob;
TEvDev *Ev;
int i, fd;

Devices=ListCreate();
/*
glob("/sys/class/input/event*", 0, 0, &Glob);
for (i=0; i < Glob.gl_pathc; i++)
{
  Path=MCopyStr(Path, Glob.gl_pathv[i], "/device/capabilities/ev", NULL);
  Tempstr=ReadFile(Tempstr, Path);
  Ev->caps=strtol(Tempstr, NULL, 16);


  Path=MCopyStr(Path, Glob.gl_pathv[i], "/device/name", NULL);
  Ev->name=ReadFile(Ev->name, Path);

  Tempstr=MCopyStr(Tempstr, "/dev/input/", GetBasename(Glob.gl_pathv[i]), NULL);
  Ev->S=STREAMOpen(Tempstr, "r");

	if (Ev->S) 
	{
	EvdevLoadAxes(Ev);
	STREAMSetItem(Ev->S, "evdev", Ev);
	}

  ListAddNamedItem(Devices, GetBasename(Glob.gl_pathv[i]), Ev);
}

*/

glob("/dev/input/event*", 0, 0, &Glob);
for (i=0; i < Glob.gl_pathc; i++)
{
  Ev=(TEvDev *) calloc(1, sizeof(TEvDev));
  Ev->S=STREAMOpen(Glob.gl_pathv[i], "r");
	if (Ev->S)
	{
	ioctl(Ev->S->in_fd, EVIOCGBIT(0, EV_MAX), &Ev->caps);
	Tempstr=SetStrLen(Tempstr, 1024);
	ioctl(Ev->S->in_fd, EVIOCGNAME(1024), Tempstr);
	Ev->name=CopyStr(Ev->name, Tempstr);
	EvdevLoadAxes(Ev);
  ListAddNamedItem(Devices, GetBasename(Glob.gl_pathv[i]), Ev);
	}
}




globfree(&Glob);
Destroy(Tempstr);
Destroy(Path);

return(Devices);
}

