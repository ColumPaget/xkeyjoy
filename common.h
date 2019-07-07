#ifndef XKEYJOY_COMMON_H
#define XKEYJOY_COMMON_H

#include "libUseful/libUseful.h"
#include <glob.h>


#define XKEYDOWN 1
#define XKEYUP   2
#define XBTNDOWN 3
#define XBTNUP   4


#define EV_XKB (EV_MAX+1)
#define EV_XBTN (EV_MAX+2)


typedef struct
{
int flags;
unsigned int intype; //button, key, axis or xkeygrab
unsigned int input; //input button, key, or axis
int inmods; //input modifiers (shift, alt, etc)
int value; //this is the value for joystick axis etc
unsigned int output; //this is the key to send
unsigned int outmods; //output modifiers (shift, alt, etc)
char *target;
} TInputMap;


int BitIsSet(void *BitMask, int BitPos);
char *ReadFile(char *RetStr, const char *Path);


#endif
