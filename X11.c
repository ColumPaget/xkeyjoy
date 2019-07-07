#include "X11.h"

#include <X11/Xlib.h> 
#include <X11/Xatom.h> 
#include <X11/Xutil.h> 
#include <X11/XF86keysym.h>
#include <X11/keysym.h>

#include "proc.h"

Display *display;
Window RootWin;
Atom WM_PID;

#define EVENT_MASK (EnterWindowMask | LeaveWindowMask | KeyPressMask | ButtonPressMask | ButtonReleaseMask)

Window X11GetFocusedWin()
{
Window focused, parent;
int trash;

XGetInputFocus(display, &focused, &trash);

return(focused);
}

Window X11GetPointerWin()
{
Window CurrWin, tmpWin;
int root_x, root_y, win_x, win_y, mask;

XQueryPointer(display, RootWin, &tmpWin, &CurrWin, &root_x, &root_y, &win_x, &win_y, &mask);	

return(CurrWin);
}



Window X11FindWin(const char *Name)
{
if (! StrValid(Name)) return(NULL);

if (strcmp(Name, "root")==0) return (RootWin);
if (strcmp(Name, "rootwin")==0) return (RootWin);

return(NULL);
}

void X11SetupEvents(Window CurrWin)
{
	XSetWindowAttributes WinAttr;

	memset(&WinAttr, 0, sizeof(WinAttr));
  WinAttr.event_mask = EVENT_MASK;
	XChangeWindowAttributes(display, CurrWin, CWEventMask, &WinAttr);

	//XSelectInput(display, CurrWin, KeyPressMask | ButtonPressMask | ButtonReleaseMask );

	XFlush(display);
}	


char *X11WindowGetCmdLine(char *RetStr, Window win)
{
unsigned long nprops=0, trash; 
unsigned char *p_Data;
int PropFormat;
Atom PType;
int len, i;
pid_t pid=0;


RetStr=CopyStr(RetStr, "");

//first try getting window pid (_NET_WM_PID property) and looking command line up from that using the /proc filesystem
XGetWindowProperty(display, win, WM_PID, 0, 1024, 0, AnyPropertyType, &PType, &PropFormat, &nprops, &trash, &p_Data);
if (nprops > 0) 
{
	pid= (pid_t) * (int *) p_Data;
	RetStr=GetProcessCmdLine(RetStr,  pid);
}


//if, for any reason, we failed to get a command-line from the window pid, start desperately trying other window properties
if (! StrValid(RetStr))
{
	//the XA_WM_COMMAND property gives us the full command-line of the app that owns the window
	XGetWindowProperty(display, win, XA_WM_COMMAND, 0, 1024, 0, XA_STRING, &PType, &PropFormat, &nprops, &trash, &p_Data);

	//if XA_WM_COMMAND doesn't work, then our last hope is to just get the window name.
	if (nprops ==0) XGetWindowProperty(display, win, XA_WM_NAME, 0, 1024, 0, XA_STRING, &PType, &PropFormat, &nprops, &trash, &p_Data);

	if (nprops > 0)
	{
		for (i=0; i < nprops-1; i++) 
		{
			if (p_Data[i]=='\0') p_Data[i]=' ';
		}
	}
	RetStr=CatStr(RetStr, p_Data);
}


return(RetStr);
}



KeySym X11TranslateKey(int key)
{
KeySym ks;
char kstring[2];

switch (key)
{
case ' ':
ks=XK_space;
break;

case TKEY_LEFT:
ks=XK_Left;
break;

case TKEY_RIGHT:
ks=XK_Right;
break;

case TKEY_UP:
ks=XK_Up;
break;

case TKEY_DOWN:
ks=XK_Down;
break;

case TKEY_PGUP:
ks=XK_Page_Up;
break;

case TKEY_PGDN:
ks=XK_Page_Down;
break;

case TKEY_HOME:
ks=XK_Home;
break;

case TKEY_END:
ks=XK_End;
break;

case TKEY_PAUSE:
ks=XK_Pause;
break;

case TKEY_INSERT:
ks=XK_Insert;
break;

case TKEY_DELETE:
ks=XK_Delete;
break;

case TKEY_WIN:
ks=XK_Super_L;
break;

case TKEY_MENU:
ks=XK_Menu;
break;

case TKEY_LSHIFT:
ks=XK_Shift_L;
break;

case TKEY_RSHIFT:
ks=XK_Shift_R;
break;

case TKEY_LCNTRL:
ks=XK_Control_L;
break;

case TKEY_RCNTRL:
ks=XK_Control_R;
break;

case TKEY_ENTER:
ks=XK_Return;
break;

case TKEY_F1:
ks=XK_F1;
break;

case TKEY_F2:
ks=XK_F2;
break;

case TKEY_F3:
ks=XK_F3;
break;

case TKEY_F4:
ks=XK_F4;
break;

case TKEY_F5:
ks=XK_F5;
break;

case TKEY_F6:
ks=XK_F6;
break;

case TKEY_F7:
ks=XK_F7;
break;

case TKEY_F8:
ks=XK_F8;
break;

case TKEY_F9:
ks=XK_F9;
break;

case TKEY_F10:
ks=XK_F10;
break;

case TKEY_F11:
ks=XK_F11;
break;

case TKEY_F12:
ks=XK_F12;
break;

case TKEY_WWW:
ks=XF86XK_WWW;
break;

case TKEY_MAIL:
ks=XF86XK_Mail;
break;

case TKEY_BACK:
ks=XF86XK_Back;
break;

case TKEY_SHOP:
ks=XF86XK_Shop;
break;

case TKEY_SEARCH:
ks=XF86XK_Search;
break;

case TKEY_FORWARD:
ks=XF86XK_Forward;
break;

case TKEY_RELOAD:
ks=XF86XK_Refresh;
break;

case TKEY_CALC:
ks=XF86XK_Calculator;
printf("CALC!      %d\n", ks);
break;

case TKEY_MYCOMPUTER:
ks=XF86XK_Calculator;
printf("MYCOMP!    %d\n", ks);
break;

case TKEY_FAVES:
ks=XF86XK_Favorites;
break;

case TKEY_LIGHTBULB:
ks=XF86XK_LightBulb;
break;

case TKEY_WAKEUP:
ks=XF86XK_WakeUp;
break;

case TKEY_SLEEP:
ks=XF86XK_Sleep;
break;


default:
if (key < 128)
{
	kstring[0]=key & 0xFF;
	kstring[1]='\0';
	ks=XStringToKeysym(kstring);
}
break;
}

printf("TRANS! %d %d\n", key, ks);
return(ks);
}





int X11TranslateKeycode(unsigned int keycode)
{
KeySym ks;
const char *ptr;

ks=XKeycodeToKeysym(display, keycode, 0);
switch (ks)
{
	case XK_Escape: return(ESCAPE);	break;
	case XK_F1: return(TKEY_F1); break;
	case XK_F2: return(TKEY_F2); break;
	case XK_F3: return(TKEY_F3); break;
	case XK_F4: return(TKEY_F4); break; 
	case XK_F5: return(TKEY_F5); break;
	case XK_F6: return(TKEY_F6); break;
	case XK_F7: return(TKEY_F7); break;
	case XK_F8: return(TKEY_F8); break;
	case XK_F9: return(TKEY_F9); break;
	case XK_F10: return(TKEY_F10); break;
	case XK_F11: return(TKEY_F11); break;
	case XK_F12: return(TKEY_F12); break;
	case XK_space: return(' '); break;
	case XK_Left: return(TKEY_LEFT); break;
	case XK_Right: return(TKEY_RIGHT); break;
	case XK_Up: return(TKEY_UP); break;
	case XK_Down: return(TKEY_DOWN); break;
	case XK_Page_Up: return(TKEY_PGUP); break;
	case XK_Page_Down: return(TKEY_PGDN); break;
	case XK_Home: return(TKEY_HOME); break;
	case XK_End: return(TKEY_END); break;
	case XK_Pause: return(TKEY_PAUSE); break;
	case XK_Insert: return(TKEY_INSERT); break;
	case XK_Delete: return(TKEY_DELETE); break;
	case XK_Super_L: return(TKEY_WIN); break;
	case XK_Menu: return(TKEY_MENU); break;
	case XK_Shift_L: return(TKEY_LSHIFT); break;
	case XK_Shift_R: return(TKEY_RSHIFT); break;
	case XK_Return: return(TKEY_ENTER); break;

	case XF86XK_WWW: return(TKEY_WWW); break;
	case XF86XK_Mail: return(TKEY_MAIL); break;
	case XF86XK_Back: return(TKEY_BACK); break;
	case XF86XK_Shop: return(TKEY_SHOP); break;
	case XF86XK_Search: return(TKEY_SEARCH); break;
	case XF86XK_Forward: return(TKEY_FORWARD); break;
	case XF86XK_Refresh: return(TKEY_RELOAD); break;
	case XF86XK_Calculator: return(TKEY_CALC); break;
	case XF86XK_MyComputer: return(TKEY_MYCOMPUTER); break;
	case XF86XK_Favorites: return(TKEY_FAVES); break;
	case XF86XK_LightBulb: return(TKEY_LIGHTBULB); break;

	case XF86XK_WakeUp: return(TKEY_WAKEUP); break;
	case XF86XK_Sleep: return(TKEY_SLEEP); break;


	default:
		ptr=XKeysymToString(ks);
		if (ptr && (StrLen(ptr) ==1) ) return(*ptr);
	break;
}

return(0);
}


void X11SendKey(Window win, unsigned int key, unsigned int mods, int state)
{
XEvent ev;

if (key==0) return;

printf("sendkey: %d target=%x root=%d\n", key, win, RootWin);

ev.xkey.state=0;
if (mods & KEYMOD_SHIFT) ev.xkey.state |= ShiftMask;
if (mods & KEYMOD_CTRL)  ev.xkey.state |= ControlMask;
if (mods & KEYMOD_ALT)   ev.xkey.state |= Mod1Mask;

switch (key)
{
case MOUSE_BTN_1:
case MOUSE_BTN_2:
case MOUSE_BTN_3:
case MOUSE_BTN_4:
case MOUSE_BTN_5:
case MOUSE_BTN_6:
case MOUSE_BTN_7:
case MOUSE_BTN_8:
case MOUSE_BTN_9:
case MOUSE_BTN_10:
case MOUSE_BTN_11:
case MOUSE_BTN_12:
	if (state) ev.type=ButtonPress;
	else ev.type=ButtonRelease;

	ev.xbutton.display=display;
	ev.xbutton.window=win;
	ev.xbutton.subwindow=win;
	ev.xbutton.root=RootWin;

	ev.xbutton.x=100;
	ev.xbutton.y=100;
	ev.xbutton.x_root=100;
	ev.xbutton.y_root=100;

	ev.xbutton.button=key-MOUSE_BTN_1 +1;
	printf("Button: %d\n", ev.xbutton.button);
break;

default:
	if (state) ev.type=KeyPress;
	else ev.type=KeyRelease;

	ev.xkey.display=display;
	ev.xkey.window=win;
	ev.xkey.subwindow=win;
	ev.xkey.root=RootWin;

	ev.xkey.keycode=XKeysymToKeycode(display, X11TranslateKey(key));
break;
}

XSendEvent(display, win, False, ButtonPressMask | ButtonReleaseMask | KeyPressMask | KeyReleaseMask, &ev);
XSync(display, True);
}



int X11ErrorHandler(Display *Disp, XErrorEvent *Event)
{
char *Tempstr=NULL;

Tempstr=SetStrLen(Tempstr, 512);
XGetErrorText(Disp, Event->error_code, Tempstr, 512);

fprintf(stderr,"X11 Error: %s\n", Tempstr);

Destroy(Tempstr);
return(FALSE);
}


int X11ReleaseKeygrabs(Window win)
{
XUngrabKey(display, AnyKey, AnyModifier, win);
}


int X11AddKeyGrab(Window win, int key)
{
int result;


result=XGrabKey(display, XKeysymToKeycode(display, X11TranslateKey(key)), None, RootWin, False, GrabModeAsync, GrabModeAsync);
X11SetupEvents(RootWin);

printf("AddKeyGrab : %d %d\n", key, result);
}

int X11AddButtonGrab(Window win, int btn)
{
int result;

result=XGrabButton(display, btn, None, RootWin, False, ButtonPressMask | ButtonReleaseMask, GrabModeAsync, GrabModeAsync, None, None);

printf("AddButton: %d %d\n", btn, result);
X11SetupEvents(win);
}



int X11ProcessEvents(TInputMap *Input)
{
XEvent ev;

Input->intype=0;
if (XPending(display))
{
	XNextEvent(display, &ev);

	switch (ev.type)
	{
		case KeyPress: 
			Input->intype=XKEYDOWN; 
			Input->input=X11TranslateKeycode(ev.xkey.keycode);
		break;

		case KeyRelease: 
			Input->intype=XKEYUP; 
			Input->input=X11TranslateKeycode(ev.xkey.keycode);
		break;

		case ButtonPress:
			Input->intype=XBTNDOWN; 
			Input->input=ev.xbutton.button;
printf("BTN P: %d\n", Input->input);
		break;

		case ButtonRelease:
			Input->intype=XBTNUP; 
			Input->input=ev.xbutton.button;
printf("BTN R: %d\n", Input->input);
		break;
	}
}

return(TRUE);
}




int X11Init()
{

  display = XOpenDisplay(getenv("DISPLAY"));
	if (display == NULL) return(-1);

	XSetErrorHandler(X11ErrorHandler);
	
	RootWin=DefaultRootWindow(display);
  WM_PID=XInternAtom(display, "_NET_WM_PID", False);
  //WM_NAME=XInternAtom(display, "WM_NAME", False);

	return(XConnectionNumber(display));
}


