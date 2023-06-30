#include "X11.h"
#include "config.h"

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/XF86keysym.h>
#include <X11/keysym.h>

#include "proc.h"

#define EVENT_MASK (EnterWindowMask | LeaveWindowMask | ButtonPressMask | ButtonReleaseMask)

#define WINSTATE_FULLSCREEN 1
#define WINSTATE_SHADED 2
#define WINSTATE_STICKY 4
#define WINSTATE_RAISED 8
#define WINSTATE_LOWERED 16
#define WINSTATE_MAX_X   32
#define WINSTATE_MAX_Y   64

Display *display;
Window RootWin;

int ButtonMask=0;


int X11WindowGetIntegerProperty(Window win, const char *Name)
{
    unsigned long nprops=0, trash;
    unsigned char *p_Data;
    int PropFormat, val=0;
    Atom Prop, PType;

    Prop=XInternAtom(display, Name, False);
    XGetWindowProperty(display, win, Prop, 0, 1024, 0, AnyPropertyType, &PType, &PropFormat, &nprops, &trash, &p_Data);
    if (nprops > 0) val=* (int *) p_Data;

    XFree(p_Data);
    return(val);
}

void X11SetupEvents(Window CurrWin)
{
    XSetWindowAttributes WinAttr;

    memset(&WinAttr, 0, sizeof(WinAttr));
    WinAttr.event_mask = EVENT_MASK;
    XChangeWindowAttributes(display, CurrWin, CWEventMask, &WinAttr);

    XSelectInput(display, CurrWin, EVENT_MASK );

    XFlush(display);
}


Window X11GetFocusedWin()
{
    Window focused, parent;
    int trash;

    XGetInputFocus(display, &focused, &trash);
    X11SetupEvents(focused);

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
    if (! StrValid(Name)) return(None);

    if (strcmp(Name, "root")==0) return (RootWin);
    if (strcmp(Name, "rootwin")==0) return (RootWin);
    if (strncmp(Name, "0x", 2)==0) return (strtol(Name+2, NULL, 16));

    return(None);
}


int X11WindowGetState(Window win)
{
    unsigned long nprops=0, trash;
    unsigned char *p_Data;
    int PropFormat, i;
    Atom WM_STATE, WM_SHADED, WM_STICKY, WM_FULLSCREEN, WM_ABOVE, WM_BELOW, WM_MAX_X, WM_MAX_Y, PType, *AtomList;
    int WinState=0;

    WM_STATE=XInternAtom(display, "_NET_WM_STATE", False);
    WM_SHADED=XInternAtom(display, "_NET_WM_STATE_SHADED", False);
    WM_STICKY=XInternAtom(display, "_NET_WM_STATE_STICKY", False);
    WM_FULLSCREEN=XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", False);
    WM_ABOVE=XInternAtom(display, "_NET_WM_STATE_ABOVE", False);
    WM_BELOW=XInternAtom(display, "_NET_WM_STATE_BELOW", False);
    WM_MAX_X=XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
    WM_MAX_Y=XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_VERT", False);

    XGetWindowProperty(display, win, WM_STATE, 0, 1024, 0, AnyPropertyType, &PType, &PropFormat, &nprops, &trash, &p_Data);

    if (nprops > 0)
    {
        AtomList=(Atom *) p_Data;
        for (i=0; i < nprops; i++)
        {
            if (AtomList[i]==WM_SHADED) WinState |= WINSTATE_SHADED;
            if (AtomList[i]==WM_STICKY) WinState |= WINSTATE_STICKY;
            if (AtomList[i]==WM_FULLSCREEN) WinState |= WINSTATE_FULLSCREEN;
            if (AtomList[i]==WM_ABOVE) WinState |= WINSTATE_RAISED;
            if (AtomList[i]==WM_BELOW) WinState |= WINSTATE_LOWERED;
            if (AtomList[i]==WM_MAX_X) WinState |= WINSTATE_MAX_X;
            if (AtomList[i]==WM_MAX_Y) WinState |= WINSTATE_MAX_Y;
        }
    }

    XFree(p_Data);
    return(WinState);
}


pid_t X11WindowGetPID(Window win)
{
    return( (pid_t) X11WindowGetIntegerProperty(win, "_NET_WM_PID"));
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
    pid=X11WindowGetPID(win);
    if (pid > 0) RetStr=GetProcessCmdLine(RetStr,  pid);


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


void X11CloseWindow(Window win, int Action)
{
    if (Action==ACT_WINKILL) XKillClient(display, win);
    else XDestroyWindow(display, win);
    XSync(display, True);
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

    case '|':
        ks=XK_bar;
        break;

    case '/':
        ks=XK_slash;
        break;

    case '\\':
        ks=XK_backslash;
        break;

    case '#':
        ks=XK_numbersign;
        break;

    case '(':
        ks=XK_parenleft;
        break;

    case ')':
        ks=XK_parenright;
        break;

    case '[':
        ks=XK_bracketleft;
        break;

    case ']':
        ks=XK_bracketright;
        break;

    case '{':
        ks=XK_braceleft;
        break;

    case '}':
        ks=XK_braceright;
        break;

    case '~':
        ks=XK_asciitilde;
        break;

    case '^':
        ks=XK_asciicircum;
        break;

    case '`':
        ks=XK_quoteleft;
        break;

    case '\'':
        ks=XK_quoteright;
        break;

    case ':':
        ks=XK_colon;
        break;

    case ';':
        ks=XK_semicolon;
        break;

    case ',':
        ks=XK_comma;
        break;

    case '.':
        ks=XK_period;
        break;

    case '?':
        ks=XK_question;
        break;

    case '!':
        ks=XK_exclam;
        break;

    case '&':
        ks=XK_ampersand;
        break;

    case '%':
        ks=XK_percent;
        break;

    case '@':
        ks=XK_at;
        break;

    case '$':
        ks=XK_dollar;
        break;

    case '_':
        ks=XK_underscore;
        break;

    case '<':
        ks=XK_less;
        break;

    case '>':
        ks=XK_greater;
        break;

    case '=':
        ks=XK_equal;
        break;

    case '+':
        ks=XK_plus;
        break;

    case '-':
        ks=XK_minus;
        break;

    case '*':
        ks=XK_asterisk;
        break;

    case ESCAPE:
        ks=XK_Escape;
        break;

#ifdef TKEY_TAB
    case TKEY_TAB:
        ks=XK_Tab;
        break;
#endif

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

    case TKEY_PAUSE:
        ks=XK_Pause;
        break;

    case TKEY_PRINT:
        ks=XK_Print;
        break;

#ifdef TKEY_SCROLL_LOCK
    case TKEY_SCROLL_LOCK:
        ks=XK_Scroll_Lock;
        break;
#endif

#ifdef TKEY_CAPS_LOCK
    case TKEY_CAPS_LOCK:
        ks=XK_Caps_Lock;
        break;
#endif

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
        break;

    case TKEY_MYCOMPUTER:
        ks=XF86XK_Calculator;
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

    case TKEY_STANDBY:
        ks=XF86XK_Standby;
        break;

    case TKEY_MEDIA:
        ks=XF86XK_AudioMedia;
        break;

    case TKEY_MEDIA_PAUSE:
        ks=XF86XK_AudioPause;
        break;

    case TKEY_MEDIA_MUTE:
        ks=XF86XK_AudioMute;
        break;

    case TKEY_MEDIA_PREV:
        ks=XF86XK_AudioPrev;
        break;

    case TKEY_MEDIA_NEXT:
        ks=XF86XK_AudioNext;
        break;

    case TKEY_MEDIA_STOP:
        ks=XF86XK_AudioStop;
        break;

    case TKEY_EJECT:
        ks=XF86XK_Eject;
        break;

    case TKEY_VOL_UP:
        ks=XF86XK_AudioRaiseVolume;
        break;

    case TKEY_VOL_DOWN:
        ks=XF86XK_AudioLowerVolume;
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

    return(ks);
}





int X11TranslateKeycode(unsigned int keycode)
{
    KeySym ks;
    const char *ptr;

#ifdef HAVE_XKBKEYCODETOKEYSYM
#include <X11/XKBlib.h>
    ks=XkbKeycodeToKeysym(display, keycode, 0, 0);
#else
    ks=XKeycodeToKeysym(display, keycode, 0);
#endif

    switch (ks)
    {
    case XK_Escape:
        return(ESCAPE);
        break;
#ifdef TKEY_TAB
    case XK_Tab:
        return(TKEY_TAB);
        break;
#endif
    case XK_Return:
        return(TKEY_ENTER);
        break;
    case XK_space:
        return(' ');
        break;

    case XK_F1:
        return(TKEY_F1);
        break;
    case XK_F2:
        return(TKEY_F2);
        break;
    case XK_F3:
        return(TKEY_F3);
        break;
    case XK_F4:
        return(TKEY_F4);
        break;
    case XK_F5:
        return(TKEY_F5);
        break;
    case XK_F6:
        return(TKEY_F6);
        break;
    case XK_F7:
        return(TKEY_F7);
        break;
    case XK_F8:
        return(TKEY_F8);
        break;
    case XK_F9:
        return(TKEY_F9);
        break;
    case XK_F10:
        return(TKEY_F10);
        break;
    case XK_F11:
        return(TKEY_F11);
        break;
    case XK_F12:
        return(TKEY_F12);
        break;
    case XK_Left:
        return(TKEY_LEFT);
        break;
    case XK_Right:
        return(TKEY_RIGHT);
        break;
    case XK_Up:
        return(TKEY_UP);
        break;
    case XK_Down:
        return(TKEY_DOWN);
        break;
    case XK_Page_Up:
        return(TKEY_PGUP);
        break;
    case XK_Page_Down:
        return(TKEY_PGDN);
        break;
    case XK_Home:
        return(TKEY_HOME);
        break;
    case XK_End:
        return(TKEY_END);
        break;
    case XK_Insert:
        return(TKEY_INSERT);
        break;
    case XK_Delete:
        return(TKEY_DELETE);
        break;
    case XK_Super_L:
        return(TKEY_WIN);
        break;
    case XK_Menu:
        return(TKEY_MENU);
        break;
    case XK_Shift_L:
        return(TKEY_LSHIFT);
        break;
    case XK_Shift_R:
        return(TKEY_RSHIFT);
        break;
    case XK_Pause:
        return(TKEY_PAUSE);
        break;
    case XK_Print:
        return(TKEY_PRINT);
        break;

#ifdef TKEY_SCROLL_LOCK
    case XK_Scroll_Lock:
        return(TKEY_SCROLL_LOCK);
        break;
#endif

#ifdef TKEY_SCROLL_LOCK
    case XK_Caps_Lock:
        return(TKEY_CAPS_LOCK);
        break;
#endif

    case XF86XK_WWW:
        return(TKEY_WWW);
        break;
    case XF86XK_Mail:
        return(TKEY_MAIL);
        break;
    case XF86XK_Back:
        return(TKEY_BACK);
        break;
    case XF86XK_Shop:
        return(TKEY_SHOP);
        break;
    case XF86XK_Search:
        return(TKEY_SEARCH);
        break;
    case XF86XK_Forward:
        return(TKEY_FORWARD);
        break;
    case XF86XK_Refresh:
        return(TKEY_RELOAD);
        break;
    case XF86XK_Calculator:
        return(TKEY_CALC);
        break;
    case XF86XK_MyComputer:
        return(TKEY_MYCOMPUTER);
        break;
    case XF86XK_Favorites:
        return(TKEY_FAVES);
        break;
    case XF86XK_LightBulb:
        return(TKEY_LIGHTBULB);
        break;

    case XF86XK_WakeUp:
        return(TKEY_WAKEUP);
        break;
    case XF86XK_Sleep:
        return(TKEY_SLEEP);
        break;
    case XF86XK_Standby:
        return(TKEY_STANDBY);
        break;


    case XF86XK_AudioMute:
        return(TKEY_MEDIA_MUTE);
        break;
    case XF86XK_AudioNext:
        return(TKEY_MEDIA_NEXT);
        break;
    case XF86XK_AudioPrev:
        return(TKEY_MEDIA_PREV);
        break;
    case XF86XK_AudioLowerVolume:
        return(TKEY_VOL_DOWN);
        break;
    case XF86XK_AudioRaiseVolume:
        return(TKEY_VOL_UP);
        break;

    case XK_bar:
        return('|');
        break;
    case XK_slash:
        return('/');
        break;
    case XK_backslash:
        return('\\');
        break;
    case XK_numbersign:
        return('#');
        break;
    case XK_parenleft:
        return('(');
        break;
    case XK_parenright:
        return(')');
        break;
    case XK_bracketleft:
        return('[');
        break;
    case XK_bracketright:
        return(']');
        break;
    case XK_braceleft:
        return('{');
        break;
    case XK_braceright:
        return('}');
        break;
    case XK_asciitilde:
        return('~');
        break;
    case XK_asciicircum:
        return('^');
        break;



    default:
        ptr=XKeysymToString(ks);
        if (ptr && (StrLen(ptr) ==1) ) return(*ptr);
        break;
    }

    return(0);
}


void X11SendKey(Window win, int key, int mods, int state)
{
    XEvent ev;
    int upper, lower;

    if (win==0) win=RootWin;
    ev.xkey.state=0;

    if (state) ev.type=KeyPress;
    else ev.type=KeyRelease;
    ev.xkey.display=display;
    ev.xkey.window=win;
    ev.xkey.subwindow=win;
    ev.xkey.root=RootWin;

    XConvertCase(key, (KeySym *) &lower, (KeySym *) &upper);
    if (key != lower) mods |= KEYMOD_SHIFT;

    if (mods & KEYMOD_SHIFT) ev.xkey.state |= ShiftMask;
    if (mods & KEYMOD_CTRL) ev.xkey.state |= ControlMask;
    if (mods & KEYMOD_ALT) ev.xkey.state |= Mod1Mask;
    if (mods & KEYMOD_ALT2) ev.xkey.state |= Mod5Mask;

    ev.xkey.keycode=XKeysymToKeycode(display, key);
    printf("SEND KEY: %d %c %d\n", state, key, key);
    XSendEvent(display, win, False, KeyPressMask | KeyReleaseMask, &ev);
    XSync(display, True);
}



void X11SendMouseButton(Window win, unsigned int button, unsigned int state)
{
    XEvent ev;
    Window root_return, win_return;
    int rx, ry, wx, wy, mask;

    if (state) ev.type=ButtonPress;
    else ev.type=ButtonRelease;

    XQueryPointer(display, RootWin, &root_return, &win_return, &rx, &ry, &wx, &wy, &mask);

    ev.xbutton.display=display;
    ev.xbutton.root=RootWin;
    ev.xbutton.window=win;
    ev.xbutton.subwindow=None;
    ev.xbutton.x=wx;
    ev.xbutton.y=wy;
    ev.xbutton.x_root=rx;
    ev.xbutton.y_root=ry;
    ev.xbutton.time=CurrentTime;
    ev.xbutton.same_screen=True;
    ev.xbutton.send_event=True;

    ev.xbutton.button=button - MOUSE_BTN_1 +1;
    if (! state)
    {
        switch (button)
        {
        case MOUSE_BTN_1:
            ev.xbutton.state = Button1MotionMask;
            break;
        case MOUSE_BTN_2:
            ev.xbutton.state = Button2MotionMask;
            break;
        case MOUSE_BTN_3:
            ev.xbutton.state = Button3MotionMask;
            break;
        case MOUSE_BTN_4:
            ev.xbutton.state = Button4MotionMask;
            break;
        case MOUSE_BTN_5:
            ev.xbutton.state = Button5MotionMask;
            break;
        }
        XUngrabPointer(display, CurrentTime);
    }
    else
    {
        ev.xbutton.state=0;
        XGrabPointer(display, RootWin, True, ButtonMotionMask | PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
    }


    if (win == RootWin)
    {
        ev.xbutton.x=rx;
        ev.xbutton.y=ry;
    }
    else XTranslateCoordinates(display, RootWin, win, rx, ry, &(ev.xbutton.x), &(ev.xbutton.y), &(ev.xbutton.subwindow));

    XSendEvent(display, win, True, ButtonPressMask | ButtonReleaseMask, &ev);
    XAllowEvents(display, AsyncPointer, CurrentTime);
    XSync(display, True);
}



void X11SendMessage(Window Win, const char *MessageType, int Value1, int Value2)
{
    XEvent event;
    Atom TypeAtom;
    static int serial=0;

    TypeAtom=XInternAtom(display, MessageType, False);

    memset( &event, 0, sizeof (XEvent) );
    event.xclient.type = ClientMessage;
    event.xclient.serial = serial++;
    event.xclient.window = Win;
    event.xclient.message_type = TypeAtom;
    event.xclient.send_event=True;
    event.xclient.format = 32;
    event.xclient.data.l[0] = Value1;
    event.xclient.data.l[1] = Value2;
    event.xclient.data.l[2] = 0;
    event.xclient.data.l[3] = 0;
    event.xclient.data.l[4] = 0;


    XSendEvent(display, RootWin, False, SubstructureRedirectMask | SubstructureNotifyMask, &event);
    XSync(display, True);
}



void X11WindowSetState(Window Win, int Action)
{
    Atom StateAtom, StateValue=None;
    int AddOrDel=TRUE, CurrState;
    XEvent event;

    CurrState=X11WindowGetState(Win);

    switch (Action)
    {
    case ACT_WINSHADE:
        if (CurrState & WINSTATE_SHADED) AddOrDel=FALSE;
        StateValue=XInternAtom(display, "_NET_WM_STATE_SHADED", False);
        break;

    case ACT_WINSTICK:
        if (CurrState & WINSTATE_STICKY) AddOrDel=FALSE;
        StateValue=XInternAtom(display, "_NET_WM_STATE_STICKY", False);
        break;

    case ACT_WINHIDE:
        StateValue=XInternAtom(display, "_NET_WM_STATE_HIDDEN", False);
        break;

    case ACT_WINFULLSCR:
        if (CurrState & WINSTATE_FULLSCREEN) AddOrDel=FALSE;
        StateValue=XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", False);
        break;

    case ACT_WINMAX_X:
        if (CurrState & WINSTATE_MAX_X) AddOrDel=FALSE;
        StateValue=XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
        break;

    case ACT_WINMAX_Y:
        if (CurrState & WINSTATE_MAX_Y) AddOrDel=FALSE;
        StateValue=XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_VERT", False);
        break;

    case ACT_WINRAISED:
        if (CurrState & WINSTATE_RAISED) AddOrDel=FALSE;
        StateValue=XInternAtom(display, "_NET_WM_STATE_ABOVE", False);
        break;

    case ACT_WINLOWERED:
        if (CurrState & WINSTATE_LOWERED) AddOrDel=FALSE;
        StateValue=XInternAtom(display, "_NET_WM_STATE_BELOW", False);
        break;
    }


    if (StateValue != None) X11SendMessage(Win, "_NET_WM_STATE", AddOrDel, (int) StateValue);
}



void X11SwitchDesktop(int Change)
{
    int no_of_desktops, desktop=0;

    no_of_desktops=X11WindowGetIntegerProperty(RootWin, "_NET_NUMBER_OF_DESKTOPS");
    desktop=X11WindowGetIntegerProperty(RootWin, "_NET_CURRENT_DESKTOP");

    if (Change == X11_NEXT_DESKTOP) desktop++;
    else if (Change == X11_PREV_DESKTOP) desktop--;
    else desktop=Change;

    if (desktop >= no_of_desktops) desktop=0;
    if (desktop < 0) desktop=no_of_desktops -1;

    printf("SWITCH DESKTOP: %d of %d\n", desktop, no_of_desktops);

    X11SendMessage(RootWin, "_NET_CURRENT_DESKTOP", desktop, time(NULL));
}


void X11ChangeDesktops(int Change)
{
    int no_of_desktops;

    no_of_desktops=X11WindowGetIntegerProperty(RootWin, "_NET_NUMBER_OF_DESKTOPS");

    if (Change == ACT_ADD_DESKTOP) no_of_desktops++;
    else if (Change == ACT_DEL_DESKTOP) no_of_desktops--;

    if (no_of_desktops > 0)
    {
        printf("CHANGE NO OF DESKTOPS: %d\n", no_of_desktops);
        X11SendMessage(RootWin, "_NET_NUMBER_OF_DESKTOPS", no_of_desktops, 0);
    }

}



void X11SendEvent(Window win, unsigned int key, unsigned int mods, int state)
{

    if (key==0) return;

    if (Config.Flags & FLAG_DEBUG) printf("sendkey: %c %d target=%x root=%x\n", key, key, win, RootWin);

    //unset these keys on each new key
    X11SendKey(win, XK_Meta_L, 0, 0);
    X11SendKey(win, XK_Alt_L, 0, 0);
    X11SendKey(win, XK_Control_L, 0, 0);
    X11SendKey(win, XK_Shift_L, 0, 0);

    //'press' modifier keys before we press our actual key/button
    if (mods & KEYMOD_SHIFT) X11SendKey(win, XK_Shift_L, 0, state);
    if (mods & KEYMOD_CTRL) X11SendKey(win, XK_Control_L, 0, state);
    if (mods & KEYMOD_ALT) X11SendKey(win, XK_Meta_L, 0, state);
    if (mods & KEYMOD_ALT2) X11SendKey(win, XK_Meta_R, 0, state);

    XSync(display, True);
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
        X11SendMouseButton(win, key, state);
        break;

    default:
        X11SendKey(win, X11TranslateKey(key), mods, state);
        break;
    }

//XSync(display, True);
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


int X11AddKeyGrab(int key, int mods)
{
    int result, modmask=None, sym;

    if (mods & KEYMOD_SHIFT) modmask |= ShiftMask;
    if (mods & KEYMOD_CTRL)  modmask |= ControlMask;
    if (mods & KEYMOD_ALT)   modmask |= Mod1Mask;
    if (mods & KEYMOD_ALT2)   modmask |= Mod5Mask;

    sym=XKeysymToKeycode(display, X11TranslateKey(key));

    if (sym >0) result=XGrabKey(display, sym, modmask, RootWin, False, GrabModeAsync, GrabModeAsync);
    if (Config.Flags & FLAG_DEBUG) printf("Setup KeyGrabs sym=%d key=%d mods=%d RootWin=%d result=%d\n", sym, key, mods, RootWin, result);

    return(result);
}

int X11AddButtonGrab(int btn)
{
    int result;

    result=XGrabButton(display, btn, None, RootWin, False, ButtonPressMask | ButtonReleaseMask, GrabModeAsync, GrabModeAsync, None, None);
    if (Config.Flags & FLAG_DEBUG) printf("Setup ButtonGrabs btn=%d RootWin=%d result=%d\n", btn, RootWin, result);

    return(result);
}


void X11SetupGrabs(TProfile *Profile)
{
    TInputMap *IMap;
    int i;

    for (i=0; i < Profile->NoOfEvents; i++)
    {
        IMap=(TInputMap *) &Profile->Events[i];

        if (IMap->active == FALSE)
        {
            if (IMap->intype==EV_XKB)
            {
                if (X11AddKeyGrab(IMap->input, IMap->inmods) > 0) IMap->active=TRUE;
            }
            if (IMap->intype==EV_XBTN)
            {
                if (X11AddButtonGrab(IMap->input - MOUSE_BTN_1 +1) > 0) IMap->active=TRUE;
            }
        }

    }
}



int X11GetEvent(TInputMap *Input)
{
    XEvent ev;
    int result;

    Input->intype=0;
    result=XPending(display);
    while (result > 0)
    {
        XNextEvent(display, &ev);

        Input->intype=0;
        switch (ev.type)
        {
        case KeyPress:
            Input->intype=EV_XKB;
            Input->value=TRUE;
            Input->input=X11TranslateKeycode(ev.xkey.keycode);
            if (Config.Flags & FLAG_DEBUG) printf("X11 keypress: %d %d \n", Input->input, ev.xkey.keycode);

            Input->inmods=0;
            if (ev.xkey.state & ShiftMask) Input->inmods |= KEYMOD_SHIFT;
            if (ev.xkey.state & ControlMask) Input->inmods |= KEYMOD_CTRL;
            if (ev.xkey.state & Mod1Mask) Input->inmods |= KEYMOD_ALT;
            if (ev.xkey.state & Mod5Mask) Input->inmods |= KEYMOD_ALT2;
            break;

        case KeyRelease:
            Input->intype=EV_XKB;
            Input->value=FALSE;
            Input->input=X11TranslateKeycode(ev.xkey.keycode);
            if (Config.Flags & FLAG_DEBUG) printf("X11 keyrelease: %d %d \n", Input->input, ev.xkey.keycode);

            Input->inmods=0;
            if (ev.xkey.state & ShiftMask) Input->inmods |= KEYMOD_SHIFT;
            if (ev.xkey.state & ControlMask) Input->inmods |= KEYMOD_CTRL;
            if (ev.xkey.state & Mod1Mask) Input->inmods |= KEYMOD_ALT;
            if (ev.xkey.state & Mod5Mask) Input->inmods |= KEYMOD_ALT2;
            break;

        case ButtonPress:
            Input->intype=EV_XBTN;
            Input->value=TRUE;
            Input->input=MOUSE_BTN_1 + ev.xbutton.button -1;
            if (Config.Flags & FLAG_DEBUG) printf("X11 buttonpress: %d %d \n", Input->input, ev.xbutton.button);
            break;

        case ButtonRelease:
            Input->intype=EV_XBTN;
            Input->value=FALSE;
            Input->input=MOUSE_BTN_1 + ev.xbutton.button -1;
            if (Config.Flags & FLAG_DEBUG) printf("X11 buttonrelease: %d %d \n", Input->input, ev.xbutton.button);
            break;

        case MotionNotify:
            ev.xmotion.window=X11GetPointerWin();
            ev.xbutton.subwindow=None;
            ev.xmotion.state |= Button1Mask;
            XSendEvent(display, ev.xmotion.window, True, PointerMotionMask | ButtonPressMask | ButtonReleaseMask | ButtonMotionMask, &ev);
            break;
        }
        result=XPending(display);
    }

    if (result==-1)
    {
        fprintf(stderr, "ERROR: Disconnected from X11 Server\n");
        return(FALSE);
    }

    return(TRUE);
}




int X11Init()
{

    display = XOpenDisplay(getenv("DISPLAY"));
    if (display == NULL)
    {
        fprintf(stderr, "ERROR: Can't connect to X11 Server\n");
        return(-1);
    }

    XSetErrorHandler(X11ErrorHandler);

    RootWin=DefaultRootWindow(display);

    return(XConnectionNumber(display));
}


