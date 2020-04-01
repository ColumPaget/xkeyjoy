#ifndef JP_X11_H
#define JP_X11_H

#include <X11/X.h>
#include "common.h"
#include "profile.h"

Window X11GetFocusedWin();
Window X11FindWin(const char *Name);
char *X11WindowGetCmdLine(char *RetStr, Window win);

void X11SendEvent(Window win, unsigned int key, unsigned int mods, int state);
int X11ProcessEvents(TInputMap *Input);

int X11Init();
void X11SetupGrabs(TProfile *Profile);


#endif

