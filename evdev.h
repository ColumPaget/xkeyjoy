#ifndef JS_EVDEV_H
#define JS_EVDEV_H

#include "common.h"
#include "linux/input.h"

typedef struct
{
int id;
int min;
int max;
} TAxis;

typedef struct
{
char *name;
unsigned int caps;
TAxis Axis[10];
STREAM *S;
} TEvDev;


void EvdevListDevices(ListNode *Devices);
void EvdevDeviceDestroy(void *Dev);
void EvdevMonitorDevice(ListNode *Devices, const char *Name);
int EvdevLoadDevices(ListNode *Devices, int Inital);
const char *EvdevLookupName(struct input_event *ev);


#endif
