#include "evdev.h"
#include "profile.h"
#include <linux/input.h>

void EvdevPrintDevice(TEvDev *Dev, const char *EventID)
{
    int axisindex;
    uint8_t abs_bitmask[ABS_MAX/8 + 1];
    float percent_deadzone;
    struct input_absinfo abs_features;
    STREAM *S;

    S=Dev->S;
    if (! S) printf("%s %s ERROR: cannot open device\n", EventID, Dev->name);
    else
    {
        memset(abs_bitmask, 0, sizeof(abs_bitmask));
        if (ioctl(S->in_fd, EVIOCGBIT(EV_ABS, sizeof(abs_bitmask)), abs_bitmask) < 0)
            perror("evdev ioctl");

        printf("%s %s ", EventID, Dev->name);
        if (BitIsSet(&(Dev->caps), EV_ABS)) printf(" abs ");
        if (BitIsSet(&(Dev->caps), EV_KEY)) printf(" key ");
        if (BitIsSet(&(Dev->caps), EV_FF)) printf(" ff ");
        if (BitIsSet(&(Dev->caps), EV_LED)) printf(" led ");
        printf("\n");

        for(axisindex = 0; axisindex < ABS_MAX; ++axisindex)
        {
            if(BitIsSet(abs_bitmask, axisindex))
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


void EvdevListDevices(ListNode *Devices)
{
    ListNode *Curr;
    TEvDev *Dev;

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
    if (ev->type==EV_SW)
    {
        switch (ev->code)
        {
        case SW_LID:
            return("sw:lid");
            break;
        case SW_RFKILL_ALL:
            return("sw:rfkill");
            break;
        case SW_DOCK:
            return("sw:dock");
            break;
        case SW_TABLET_MODE:
            return("sw:tablet");
            break;
        }
    }
    else
    {
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
    }


    return("");
}



void EvdevMonitorDevice(ListNode *Devices, const char *Name)
{
    ListNode *Node;
    struct input_event ev;
    struct input_id evdev_id;
    char *Tempstr=NULL, *Buttons=NULL;
    STREAM *StdOut;
    TEvDev *Dev;
    int Axis[10];
    int i, result;

    Node=ListFindNamedItem(Devices, Name);
    if (Node)
    {
        Dev=(TEvDev *) Node->Item;
        memset(Axis, 0, sizeof(Axis));
        StdOut=STREAMFromFD(1);
        TerminalClear(StdOut);
        TerminalCursorMove(StdOut, 0, 0);
        ioctl(Dev->S->in_fd, EVIOCGID, &evdev_id);
        Tempstr=FormatStr(Tempstr, "%s: %s vendor=%d product=%d version=%d~>", Name, Dev->name, evdev_id.vendor, evdev_id.product, evdev_id.version);
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
                    Buttons=FormatStr(Buttons, "press:  type=%u code=%x value=%x", ev.type, ev.code, ev.value);
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



int EvdevLoadDevices(ListNode *Devices)
{
    char *Tempstr=NULL, *Path=NULL, *Name=NULL;
    glob_t Glob;
    TEvDev *Ev;
    STREAM *S;
    int i, fd, New=FALSE;


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
                if (Flags & FLAG_DEBUG) printf("ADD: %s\n", Ev->S->Path);
                ioctl(Ev->S->in_fd, EVIOCGBIT(0, EV_MAX), &Ev->caps);
                Tempstr=SetStrLen(Tempstr, 1024);
                ioctl(Ev->S->in_fd, EVIOCGNAME(1024), Tempstr);
                Ev->name=CopyStr(Ev->name, Tempstr);
                EvdevLoadAxes(Ev);
                ListAddNamedItem(Devices, Name, Ev);
                New=TRUE;
            }
        }
    }


    globfree(&Glob);

    Destroy(Tempstr);
    Destroy(Path);
    Destroy(Name);

    return(New);
}

void EvdevDeviceDestroy(void *p_Dev)
{
    TEvDev *Dev;

    Dev=(TEvDev *) p_Dev;
    Destroy(Dev->name);
    STREAMClose(Dev->S);
    Destroy(Dev);
}
