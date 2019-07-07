#ifndef JP_X11_H
#define JP_X11_H

#include <X11/X.h>
#include "common.h"


Window X11GetFocusedWin();
Window X11FindWin(const char *Name);
char *X11WindowGetCmdLine(char *RetStr, Window win);

void X11SendKey(Window win, unsigned int key, unsigned int mods, int state);
int X11ProcessEvents(TInputMap *Input);

int X11ReleaseKeygrabs(Window win);
int X11AddKeyGrab(Window win, int key);
int X11AddButtonGrab(Window win, int btn);
int X11Init();

#endif

