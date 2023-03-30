#ifndef JS_EVDEV_H
#define JS_EVDEV_H

#include "common.h"
#include "linux/input.h"

typedef enum {EDSTATE_RUNNING, EDSTATE_INITIAL, EDSTATE_LIST} EDevStates;
typedef enum {DT_KEYBOARD, DT_MOUSE, DT_TRACKPAD, DT_TOUCHSCREEN, DT_TABLET, DT_GAMEPAD, DT_SWITCH, DT_OTHER} EDevTypes;

typedef struct
{
int id;
int min;
int max;
} TAxis;

typedef struct
{
char *name;
int devtype;
uint64_t caps; //uint64 should be enough space to hold all caps
TAxis Axis[10];
STREAM *S;
} TEvDev;


void EvdevListDevices();
void EvdevDeviceDestroy(void *Dev);
void EvdevMonitorDevice(const char *Name);
int EvdevLoadDevices(ListNode *Devices, int State);
const char *EvdevLookupName(struct input_event *ev);
void EvdevRemoveDevice(ListNode *Devices, STREAM *S);

#endif
