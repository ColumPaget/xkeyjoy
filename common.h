#ifndef XKEYJOY_COMMON_H
#define XKEYJOY_COMMON_H

#ifdef HAVE_LIBUSEFUL5
#include "libUseful-5/libUseful.h"
#else
#include "libUseful-4/libUseful.h"
#endif


#include <glob.h>


#define XKEYDOWN 1
#define XKEYUP   2
#define XBTNDOWN 3
#define XBTNUP   4

typedef enum {ACT_SENDKEY, ACT_EXEC, ACT_WINCLOSE, ACT_WINKILL, ACT_WINHIDE, ACT_WINSHADE, ACT_WINSTICK, ACT_WINFULLSCR, ACT_WINMAX_X, ACT_WINMAX_Y, ACT_WINRAISED, ACT_WINLOWERED, ACT_SWITCH_DESKTOP, ACT_NEXT_DESKTOP, ACT_PREV_DESKTOP, ACT_ADD_DESKTOP, ACT_DEL_DESKTOP} EActions;

#define EV_XKB (EV_MAX+1)
#define EV_XBTN (EV_MAX+2)


typedef struct
{
int flags;
int active;
unsigned int intype; //button, key, axis or xkeygrab
unsigned int input; //input button, key, or axis
int inmods; //input modifiers (shift, alt, etc)
int value; //this is the value for joystick axis etc
unsigned int action; //action to perform: sendkey, exec program, kill pid, etc
unsigned int output; //this is the key to send
unsigned int outmods; //output modifiers (shift, alt, etc)
char *target;
} TInputMap;

int BitIsSet(void *BitMask, int BitPos, int size);
char *ReadFile(char *RetStr, const char *Path);


#endif
