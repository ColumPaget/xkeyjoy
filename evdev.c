#include "evdev.h"
#include "profile.h"
#include "config.h"
#include <linux/input.h>


int EvdevGetType(uint64_t *caps, uint64_t *props)
{
    int devtype=DT_OTHER;

    if (BitIsSet(caps, EV_SW, sizeof(uint64_t))) devtype=DT_SWITCH;
    if (BitIsSet(caps, EV_ABS, sizeof(uint64_t))) devtype=DT_GAMEPAD;
    if (BitIsSet(caps, EV_REP, sizeof(uint64_t))) devtype=DT_KEYBOARD;

    if (BitIsSet(props, INPUT_PROP_POINTER, sizeof(uint64_t))) devtype=DT_TRACKPAD;
    if (BitIsSet(props, INPUT_PROP_DIRECT, sizeof(uint64_t))) 
    {
       if (devtype == DT_TRACKPAD) devtype = DT_TABLET;
       else devtype=DT_TOUCHSCREEN;
    }

//only mice have EV_REL
    if (BitIsSet(caps, EV_REL, sizeof(uint64_t))) devtype=DT_MOUSE;

    return(devtype);
}


const char *EvdevGetTypeLabel(int devtype)
{
    switch (devtype)
    {
    case DT_MOUSE:
        return("mouse");
        break;
    case DT_TRACKPAD:
        return("trackpad");
        break;
    case DT_TABLET:
        return("tablet");
        break;
    case DT_TOUCHSCREEN:
        return("touchscreen");
        break;
    case DT_GAMEPAD:
        return("game");
        break;
    case DT_KEYBOARD:
        return("keyboard");
        break;
    case DT_SWITCH:
	return("switch");
	break;
    }

    return("other");
}



char *EvdevGetName(char *RetStr, const char *Path)
{
    const char *p_Name;
    char *Tempstr=NULL;
    STREAM *S;

    RetStr=CopyStr(RetStr, "????");
    p_Name=GetBasename(Path);
    Tempstr=MCopyStr(Tempstr, "/sys/class/input/", p_Name, "/device/name", NULL);
    S=STREAMOpen(Tempstr, "r");
    if (S)
    {
        RetStr=STREAMReadLine(RetStr, S);
        StripTrailingWhitespace(RetStr);
        STREAMClose(S);
    }

    Destroy(Tempstr);
    return(RetStr);
}


char *EvdevGetNameFromSTREAM(char *RetStr, STREAM *S)
{
    char *Tempstr=NULL;

    Tempstr=SetStrLen(Tempstr, 1024);
    ioctl(S->in_fd, EVIOCGNAME(1024), Tempstr);
    RetStr=CopyStr(RetStr, Tempstr);

    Destroy(Tempstr);

    return(RetStr);
}


char *EvdevFormatDevDetails(char *RetStr, STREAM *S)
{
    struct input_id evdev_id;

    ioctl(S->in_fd, EVIOCGID, &evdev_id);
    RetStr=FormatStr(RetStr, "vendor=%04d product=%04d version=%04d", evdev_id.vendor, evdev_id.product, evdev_id.version);

    return(RetStr);
}


#define IGNORE 0
#define YES    1
#define NO     2


int EvdevConsiderList(const char *List, const char *Item)
{
    int result=IGNORE;
    char *Token=NULL;
    const char *ptr;

    ptr=GetToken(List, ",", &Token, 0);
    while (ptr)
    {
        //if the item is in the list, then it's a 'yes'
        if (strcasecmp(Token, Item)==0)
        {
            Destroy(Token);
            return(YES);
        }

        if (*Token=='!')
        {
            //if !item  is in the list, then it's a 'no'
            if (strcasecmp(Token+1, Item)==0)
            {
                Destroy(Token);
                return(NO);
            }

            //if we say '!item' then any non-matching item is a yes
            result=YES;
        }

        ptr=GetToken(ptr, ",", &Token, 0);
    }

    Destroy(Token);
    return(result);
}


//decide if
int EvdevUseDevice(TEvDev *Dev, int State)
{
    char *Token=NULL;
    const char *ptr;
    int result=IGNORE, RetVal=FALSE;

    if (State==EDSTATE_LIST) return(TRUE);

    if (
        (! StrValid(Config.Devices)) &&
        (! StrValid(Config.DevTypes))
    ) return(TRUE);

    result=EvdevConsiderList(Config.DevTypes, EvdevGetTypeLabel(Dev->devtype));
    if (result==YES) RetVal=TRUE;
    if (result==NO) RetVal=FALSE;

    result=EvdevConsiderList(Config.Devices, Dev->name);

    if (result==YES) RetVal=TRUE;
    if (result==NO) RetVal=FALSE;


    Destroy(Token);

    return(RetVal);
}


void EvdevPrintDevice(TEvDev *Dev, const char *EventID)
{
    int axisindex, props;
    uint8_t abs_bitmask[ABS_MAX/8 + 1];
    float percent_deadzone;
    struct input_absinfo abs_features;
    STREAM *S;

    S=Dev->S;
    if (! S) printf("%s %s ERROR: cannot open device\n", EventID, Dev->name);
    else
    {
        memset(abs_bitmask, 0, sizeof(abs_bitmask));
        if (ioctl(S->in_fd, EVIOCGBIT(EV_ABS, sizeof(abs_bitmask)), abs_bitmask) < 0) perror("evdev ioctl");


        printf("%s %s {%s} ", EventID, EvdevGetTypeLabel(Dev->devtype), Dev->name);
        if (BitIsSet(&(Dev->caps), EV_ABS, sizeof(uint64_t))) printf(" abs ");
        if (BitIsSet(&(Dev->caps), EV_KEY, sizeof(uint64_t))) printf(" key ");
        if (BitIsSet(&(Dev->caps), EV_FF, sizeof(uint64_t))) printf(" ff ");
        if (BitIsSet(&(Dev->caps), EV_LED, sizeof(uint64_t))) printf(" led ");
        if (BitIsSet(&(Dev->caps), EV_REL, sizeof(uint64_t))) printf(" rel ");
        if (BitIsSet(&(Dev->caps), EV_REP, sizeof(uint64_t))) printf(" rep ");
        if (BitIsSet(&(Dev->caps), EV_SW, sizeof(uint64_t))) printf(" sw ");
        if (BitIsSet(&(Dev->caps), EV_PWR, sizeof(uint64_t))) printf(" pwr ");
        if (BitIsSet(&(Dev->caps), EV_SND, sizeof(uint64_t))) printf(" snd ");
        printf("\n");

        for(axisindex = 0; axisindex < ABS_MAX; ++axisindex)
        {
            if(BitIsSet(abs_bitmask, axisindex, sizeof(abs_bitmask)))
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


void EvdevListDevices()
{
    ListNode *Curr;
    TEvDev *Dev;
    ListNode *Devices;

    Devices=ListCreate();
    EvdevLoadDevices(Devices, EDSTATE_LIST);

    Curr=ListGetNext(Devices);
    while (Curr)
    {
        Dev=(TEvDev *) Curr->Item;
        EvdevPrintDevice(Dev, Curr->Tag);
        Curr=ListGetNext(Curr);
    }
}


const char *EvdevLookupName(struct input_event *ev)
{
    switch (ev->type)
    {
    case EV_SW:
        switch (ev->code)
        {
        case SW_DOCK:
            return("sw:dock");
            break;
        case SW_HEADPHONE_INSERT:
            return("sw:headphone");
            break;
        case SW_LINEIN_INSERT:
            return("sw:linein");
            break;
        case SW_LINEOUT_INSERT:
            return("sw:lineout");
            break;
        case SW_LID:
            return("sw:lid");
            break;
        case SW_RFKILL_ALL:
            return("sw:rfkill");
            break;
        case SW_TABLET_MODE:
            return("sw:tablet");
            break;
        }
        break;

    case EV_KEY:
        switch (ev->code)
        {
        case BTN_A:
            return("btn:A");
            break;
        case BTN_B:
            return("btn:B");
            break;
        case BTN_C:
            return("btn:C");
            break;
        case BTN_X:
            return("btn:X");
            break;
        case BTN_Y:
            return("btn:Y");
            break;
        case BTN_Z:
            return("btn:Z");
            break;
        case BTN_SELECT:
            return("btn:select");
            break;
        case BTN_START:
            return("btn:start");
            break;
        case BTN_MODE:
            return("btn:mode");
            break;
        case BTN_TL:
            return("btn:ltrig");
            break;
        case BTN_TR:
            return("btn:rtrig");
            break;
        case BTN_DPAD_UP:
            return("btn:up");
            break;
        case BTN_DPAD_DOWN:
            return("btn:down");
            break;
        case BTN_DPAD_LEFT:
            return("btn:left");
            break;
        case BTN_DPAD_RIGHT:
            return("btn:right");
            break;
        }
        break;

    case EV_ABS:
        return("joystick");
        break;
    }


    return("");
}



void EvdevMonitorDevice(const char *Name)
{
    ListNode *Node;
    struct input_event ev;
    char *Tempstr=NULL, *Buttons=NULL;
    STREAM *StdOut;
    TEvDev *Dev;
    int Axis[10];
    int i, result;
    ListNode *Devices;

    Devices=ListCreate();
    EvdevLoadDevices(Devices, EDSTATE_LIST);


    Node=ListFindNamedItem(Devices, Name);
    if (Node)
    {
        Dev=(TEvDev *) Node->Item;
        memset(Axis, 0, sizeof(Axis));
        StdOut=STREAMFromFD(1);
        TerminalClear(StdOut);
        TerminalCursorMove(StdOut, 0, 0);
        Tempstr=MCopyStr(Tempstr, Dev->S->Path, "  ", Dev->name, "~>", NULL);
        TerminalPutStr(Tempstr, StdOut);

        while (1)
        {
            Buttons=CopyStr(Buttons, "");
            while (STREAMCheckForBytes(Dev->S))
            {
                result=STREAMReadBytes(Dev->S, (char *) &ev, sizeof(struct input_event));

                switch (ev.type)
                {
                case EV_ABS:
                    Axis[ev.code]=ev.value;
                    break;

                case EV_SW:
                case EV_KEY:
                    Buttons=FormatStr(Buttons, "press:  type=%u code=%x value=%x ", ev.type, ev.code, ev.value);
                    Buttons=MCatStr(Buttons, EvdevLookupName(&ev), " ", NULL);
                    break;
                }

            }


            if (StrValid(Buttons))
            {
                TerminalCursorMove(StdOut, 0, 2);
                Buttons=CatStr(Buttons, "~>");
                TerminalPutStr(Buttons, StdOut);
            }

            for (i=0; i < 6; i++)
            {
                TerminalCursorMove(StdOut, 0, i+3);
                Tempstr=FormatStr(Tempstr, "axis: %d  %d~>", i, Axis[i]);
                TerminalPutStr(Tempstr, StdOut);
            }

            STREAMFlush(StdOut);
            usleep(20000);
        }
    }

    Destroy(Tempstr);
    Destroy(Buttons);
}



void EvdevLoadAxes(TEvDev *Ev)
{
    struct input_absinfo abs_features;
    float percent_deadzone=0;
    int i;

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



int EvdevLoadDevices(ListNode *Devices, int State)
{
    char *Tempstr=NULL, *Name=NULL;
    glob_t Glob;
    TEvDev *Ev;
    STREAM *S;
    uint64_t prop_bitmask;
    int i, New=FALSE;

    if (Config.Flags & FLAG_NOEVDEV) return(FALSE);

    glob("/dev/input/event*", 0, 0, &Glob);
    for (i=0; i < Glob.gl_pathc; i++)
    {
        Name=CopyStr(Name, GetBasename(Glob.gl_pathv[i]));
        if (! ListFindNamedItem(Devices, Name))
        {
            S=STREAMOpen(Glob.gl_pathv[i], "r");
            if (S)
            {
                Ev=(TEvDev *) calloc(1, sizeof(TEvDev));
                Ev->S=S;
                Ev->name=EvdevGetNameFromSTREAM(Ev->name, Ev->S);
                ioctl(Ev->S->in_fd, EVIOCGBIT(0, EV_MAX), &Ev->caps);

                ioctl(S->in_fd, EVIOCGPROP(sizeof(prop_bitmask)), &prop_bitmask);
                Ev->devtype=EvdevGetType(&Ev->caps, &prop_bitmask);

                EvdevLoadAxes(Ev);

                if (EvdevUseDevice(Ev, State))
                {
                    ListAddNamedItem(Devices, Name, Ev);
                    New=TRUE;

                    if ((State == EDSTATE_INITIAL) && (Config.Flags & FLAG_DEBUG) )
                    {
                        Tempstr=EvdevFormatDevDetails(Tempstr, Ev->S);
                        printf("ADD: %s %s %s\n", Ev->S->Path, Tempstr, Ev->name);
                    }
                }
                else
                {
                    if ((State == EDSTATE_INITIAL) && (Config.Flags & FLAG_DEBUG) )
                    {
                        Tempstr=EvdevFormatDevDetails(Tempstr, Ev->S);
                        printf("IGNORE: %s %s %s\n", Ev->S->Path, Tempstr, Ev->name);
                    }
                    EvdevDeviceDestroy(Ev);
                }
            }
            else
            {
                Tempstr=EvdevGetName(Tempstr, Glob.gl_pathv[i]);
                if ((State == EDSTATE_INITIAL) && (Config.Flags & FLAG_DEBUG)) printf("FAILED TO OPEN: %s %s\n", Glob.gl_pathv[i], Tempstr);
            }
        }
    }
    globfree(&Glob);



    Destroy(Tempstr);
    Destroy(Name);

    return(New);
}




void EvdevDeviceDestroy(void *p_Dev)
{
    TEvDev *Dev;

    Dev=(TEvDev *) p_Dev;
    Destroy(Dev->name);
    //Do not close stream, it will be closed elsewhere
    //STREAMClose(Dev->S);
    Destroy(Dev);
}


void EvdevRemoveDevice(ListNode *Devices, STREAM *S)
{
    ListNode *Curr, *Next;
    TEvDev *Dev;


    Curr=ListGetNext(Devices);
    while (Curr)
    {
        Next=ListGetNext(Curr);
        Dev=(TEvDev *) Curr->Item;
        if (Dev->S == S)
        {
            ListDeleteNode(Curr);
            EvdevDeviceDestroy(Dev);
        }
        Curr=Next;
    }

}
