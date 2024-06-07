#ifndef JP_X11_H
#define JP_X11_H

#include <X11/X.h>
#include "common.h"
#include "profile.h"

#define X11_PREV_DESKTOP -1
#define X11_NEXT_DESKTOP -2

Window X11GetFocusedWin();
Window X11GetPointerWin();
Window X11FindWin(const char *Name);
pid_t X11WindowGetPID(Window win);
char *X11WindowGetCmdLine(char *RetStr, Window win);
int X11WindowGetState(Window win);
void X11WindowSetState(Window Win, int State);
void X11SendEvent(Window win, unsigned int key, unsigned int mods, int state);
int X11GetEvent(TInputMap *Input);
void X11CloseWindow(Window win, int Action);
void X11SwitchDesktop(int Change);
void X11ChangeDesktops(int Change);
int X11Init();
void X11SetupGrabs(TProfile *Profile);

//maybe one day
//void X11SwitchVT(Window target, int VT);

#endif

