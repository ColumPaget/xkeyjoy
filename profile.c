#include <linux/input.h>
#include "profile.h"
#include "proc.h"
#include <glob.h>
#include <fnmatch.h>

static ListNode *Profiles=NULL;
static TProfile *Grabs=NULL;
static char *AddToAllProfiles=NULL;

void ProfileDestroy(void *p_Profile)
{
    TProfile *Profile;

    if (! p_Profile) return;
    Profile=(TProfile *) p_Profile;
    Destroy(Profile->Apps);
    Destroy(Profile->Devices);
    Destroy(Profile->Events);
    free(Profile);
}

static void ProfileParseButtonInput(TInputMap *IMap, const char *Name)
{
    IMap->intype=EV_KEY;
    if (strcasecmp(Name, "btn:A")==0) IMap->input=BTN_A;
    if (strcasecmp(Name, "btn:B")==0) IMap->input=BTN_B;
    if (strcasecmp(Name, "btn:C")==0) IMap->input=BTN_C;
    if (strcasecmp(Name, "btn:X")==0) IMap->input=BTN_X;
    if (strcasecmp(Name, "btn:Y")==0) IMap->input=BTN_Y;
    if (strcasecmp(Name, "btn:Z")==0) IMap->input=BTN_Z;
    if (strcasecmp(Name, "btn:sel")==0) IMap->input=BTN_SELECT;
    if (strcasecmp(Name, "btn:select")==0) IMap->input=BTN_SELECT;
    if (strcasecmp(Name, "btn:start")==0) IMap->input=BTN_START;
    if (strcasecmp(Name, "btn:mode")==0) IMap->input=BTN_MODE;
    if (strcasecmp(Name, "btn:tl")==0) IMap->input=BTN_TL;
    if (strcasecmp(Name, "btn:tr")==0) IMap->input=BTN_TR;
    if (strcasecmp(Name, "btn:ltrig")==0) IMap->input=BTN_TL;
    if (strcasecmp(Name, "btn:rtrig")==0) IMap->input=BTN_TR;


    if (strcasecmp(Name, "btn:up")==0) IMap->input=BTN_DPAD_UP;
    if (strcasecmp(Name, "btn:down")==0) IMap->input=BTN_DPAD_DOWN;
    if (strcasecmp(Name, "btn:left")==0) IMap->input=BTN_DPAD_LEFT;
    if (strcasecmp(Name, "btn:right")==0) IMap->input=BTN_DPAD_RIGHT;

    IMap->value=1;
}


static void ProfileParseSwitchInput(TInputMap *IMap, const char *Name)
{
    IMap->intype=EV_SW;

    if (strcasecmp(Name, "sw:lid")==0)
    {
        IMap->input=SW_LID;
        IMap->value=1;
    }
    if (strcasecmp(Name, "sw:rfkill")==0)
    {
        IMap->input=SW_RFKILL_ALL;
        IMap->value=1;
    }
    if (strcasecmp(Name, "sw:dock")==0)
    {
        IMap->input=SW_DOCK;
        IMap->value=1;
    }
    if (strcasecmp(Name, "sw:tablet")==0)
    {
        IMap->input=SW_TABLET_MODE;
        IMap->value=1;
    }
    if (strcasecmp(Name, "sw:headphone")==0)
    {
        IMap->input=SW_HEADPHONE_INSERT;
        IMap->value=1;
    }
    if (strcasecmp(Name, "sw:phones")==0)
    {
        IMap->input=SW_HEADPHONE_INSERT;
        IMap->value=1;
    }
    if (strcasecmp(Name, "sw:mic")==0)
    {
        IMap->input=SW_MICROPHONE_INSERT;
        IMap->value=1;
    }
    if (strcasecmp(Name, "sw:microphone")==0)
    {
        IMap->input=SW_MICROPHONE_INSERT;
        IMap->value=1;
    }
    if (strcasecmp(Name, "sw:lineout")==0)
    {
        IMap->input=SW_LINEOUT_INSERT;
        IMap->value=1;
    }
    if (strcasecmp(Name, "sw:linein")==0)
    {
        IMap->input=SW_LINEIN_INSERT;
        IMap->value=1;
    }


    if (strcasecmp(Name, "sw:lid-off")==0)
    {
        IMap->input=SW_LID;
        IMap->value=0;
    }
    if (strcasecmp(Name, "sw:rfkill-off")==0)
    {
        IMap->input=SW_RFKILL_ALL;
        IMap->value=0;
    }
    if (strcasecmp(Name, "sw:dock-off")==0)
    {
        IMap->input=SW_DOCK;
        IMap->value=0;
    }
    if (strcasecmp(Name, "sw:tablet-off")==0)
    {
        IMap->input=SW_TABLET_MODE;
        IMap->value=0;
    }
    if (strcasecmp(Name, "sw:headphone-off")==0)
    {
        IMap->input=SW_HEADPHONE_INSERT;
        IMap->value=0;
    }
    if (strcasecmp(Name, "sw:phones-off")==0)
    {
        IMap->input=SW_HEADPHONE_INSERT;
        IMap->value=0;
    }
    if (strcasecmp(Name, "sw:mic-off")==0)
    {
        IMap->input=SW_MICROPHONE_INSERT;
        IMap->value=0;
    }
    if (strcasecmp(Name, "sw:microphone-off")==0)
    {
        IMap->input=SW_MICROPHONE_INSERT;
        IMap->value=0;
    }
    if (strcasecmp(Name, "sw:lineout-off")==0)
    {
        IMap->input=SW_LINEOUT_INSERT;
        IMap->value=0;
    }
    if (strcasecmp(Name, "sw:linein-off")==0)
    {
        IMap->input=SW_LINEIN_INSERT;
        IMap->value=0;
    }




}


static void ProfileParseAbsInput(TInputMap *IMap, const char *Def)
{
    char *Name=NULL, *Op=NULL;
    const char *ptr;

    IMap->intype=EV_ABS;

    ptr=GetToken(Def, "<|>", &Name, GETTOKEN_MULTI_SEP | GETTOKEN_INCLUDE_SEP);
    ptr=GetToken(ptr, "<|>", &Op, GETTOKEN_MULTI_SEP | GETTOKEN_INCLUDE_SEP);

    if (ptr)
    {
        if (strcasecmp(Name, "abs0")==0) IMap->input=ABS_X;
        else if (strcasecmp(Name, "abs1")==0) IMap->input=ABS_Y;
        else if (strcasecmp(Name, "abs2")==0) IMap->input=ABS_Z;
        else if (strcasecmp(Name, "abs3")==0) IMap->input=ABS_RX;
        else if (strcasecmp(Name, "abs4")==0) IMap->input=ABS_RY;
        else if (strcasecmp(Name, "abs5")==0) IMap->input=ABS_RZ;

        if (strcmp(Op, "<")==0) IMap->flags |= ABS_LESS;
        else IMap->flags |= ABS_MORE;
        IMap->value=strtol(ptr,NULL,10);
    }

    Destroy(Name);
    Destroy(Op);
}




static unsigned int ProfileParseKey(const char *Value, int *mods)
{
    int kv;

    if (mods != NULL) *mods=0; //make sure this doesn't inherit values from elsewhere if mods not set

    if (! StrValid(Value)) return(0);

    if (strncasecmp(Value,"xbtn:",5)==0)
    {
        if (strcasecmp(Value, "xbtn:left")==0) return(MOUSE_BTN_1);
        if (strcasecmp(Value, "xbtn:middle")==0) return(MOUSE_BTN_2);
        if (strcasecmp(Value, "xbtn:right")==0) return(MOUSE_BTN_3);

        if (strcasecmp(Value, "xbtn:1")==0) return(MOUSE_BTN_1);
        if (strcasecmp(Value, "xbtn:2")==0) return(MOUSE_BTN_2);
        if (strcasecmp(Value, "xbtn:3")==0) return(MOUSE_BTN_3);
        if (strcasecmp(Value, "xbtn:4")==0) return(MOUSE_BTN_4);
        if (strcasecmp(Value, "xbtn:5")==0) return(MOUSE_BTN_5);
        if (strcasecmp(Value, "xbtn:6")==0) return(MOUSE_BTN_6);
        if (strcasecmp(Value, "xbtn:7")==0) return(MOUSE_BTN_7);
        if (strcasecmp(Value, "xbtn:8")==0) return(MOUSE_BTN_8);
        if (strcasecmp(Value, "xbtn:9")==0) return(MOUSE_BTN_9);
        if (strcasecmp(Value, "xbtn:10")==0) return(MOUSE_BTN_10);
        if (strcasecmp(Value, "xbtn:11")==0) return(MOUSE_BTN_11);
        if (strcasecmp(Value, "xbtn:12")==0) return(MOUSE_BTN_12);
        if (strcasecmp(Value, "xbtn:13")==0) return(MOUSE_BTN_13);
        if (strcasecmp(Value, "xbtn:14")==0) return(MOUSE_BTN_14);
    }

    kv=TerminalTranslateKeyStrWithMod(Value, mods);
    return(kv);
}


//In addition to adding a grab to a profile, we add it to a global list of all grabs.
//This is because we don't change our grabs when we change window, instead we listen to all
//grabs all the time, then decide what action to take when they arrive, depending on whether
//a window has a grab active or not
static void ProfileAddGrab(TInputMap *IMap)
{
    int i;
    TInputMap *Grab;

    for (i=0; i < Grabs->NoOfEvents; i++)
    {
        Grab=Grabs->Events + i;
        if (
            (Grab->intype==IMap->intype) &&
            (Grab->input==IMap->input) &&
            (Grab->inmods==IMap->inmods)
        ) return;
    }

    Grabs->Events=(TInputMap *) realloc(Grabs->Events, (Grabs->NoOfEvents+1) * sizeof(TInputMap));
    Grab=Grabs->Events + i;
    Grab->active=FALSE;
    Grab->intype=IMap->intype;
    Grab->input=IMap->input;
    Grab->inmods=IMap->inmods;

    Grabs->NoOfEvents++;
}

static void ProfileParseXKBInput(TInputMap *IMap, const char *Def)
{
    IMap->intype=EV_XKB;
    IMap->input=ProfileParseKey(Def+4, &IMap->inmods);
    ProfileAddGrab(IMap);
}

static void ProfileParseXButtonInput(TInputMap *IMap, const char *Def)
{
    IMap->intype=EV_XBTN;
    IMap->input=ProfileParseKey(Def, NULL);
    ProfileAddGrab(IMap);
}


static void ProfileKeyAddAlternatives(TProfile *Profile, TInputMap *IMap)
{
    TInputMap *Alt;

    if (IMap->intype==EV_KEY)
    {
        Alt=&Profile->Events[Profile->NoOfEvents];
        switch (IMap->input)
        {
        case BTN_DPAD_UP:
            Alt->intype=EV_ABS;
            Alt->input=ABS_HAT0Y;
            Alt->value=0;
            Alt->flags |= ABS_LESS;
            Alt->output=IMap->output;
            break;

        case BTN_DPAD_DOWN:
            Alt->intype=EV_ABS;
            Alt->input=ABS_HAT0Y;
            Alt->value=0;
            Alt->flags |= ABS_MORE;
            Alt->output=IMap->output;
            break;

        case BTN_DPAD_LEFT:
            Alt->intype=EV_ABS;
            Alt->input=ABS_HAT0X;
            Alt->flags |= ABS_LESS;
            Alt->value=0;
            Alt->output=IMap->output;
            break;

        case BTN_DPAD_RIGHT:
            Alt->intype=EV_ABS;
            Alt->input=ABS_HAT0X;
            Alt->flags |= ABS_MORE;
            Alt->value=0;
            Alt->output=IMap->output;
            break;
        }
        Profile->NoOfEvents++;
    }

}

static char *ProfilePreProcess(char *RetStr, const char *Config)
{
    char *Name=NULL, *Value=NULL, *Tempstr=NULL;
    const char *ptr;

    RetStr=CopyStr(RetStr, "");
    Tempstr=MCopyStr(Tempstr, Config, " ", AddToAllProfiles, " ", NULL);
    ptr=GetNameValuePair(Tempstr, "\\S", "=", &Name, &Value);
    while (ptr)
    {
        if (strcasecmp(Name, "dpad")==0)
        {
            if (strcasecmp(Value, "arrows")==0)
            {
                RetStr=CatStr(RetStr, "btn:left=left btn:right=right btn:up=up btn:down=down ");
            }
        }
        else RetStr=MCatStr(RetStr, Name, "=", Value, " ", NULL);
        ptr=GetNameValuePair(ptr, "\\S", "=", &Name, &Value);
    }

    Destroy(Tempstr);
    Destroy(Name);
    Destroy(Value);

    return(RetStr);
}



static int ProfileParseInputMapping(TInputMap *IMap, const char *Name, const char *Value)
{
    const char *ptr=NULL;

    if ( strncmp(Name, "abs", 3)==0) ProfileParseAbsInput(IMap, Name);
    else if (strncmp(Name, "btn:", 4)==0) ProfileParseButtonInput(IMap, Name);
    else if (strncmp(Name, "sw:", 3)==0) ProfileParseSwitchInput(IMap, Name);
    else if (strncmp(Name, "xkb:", 4)==0) ProfileParseXKBInput(IMap, Name);
    else if (strncmp(Name, "xbtn:", 5)==0) ProfileParseXButtonInput(IMap, Name);
    else return(FALSE);

    if (strncmp(Value, "rootwin:", 8)==0)
    {
        IMap->target=CopyStr(IMap->target, "root");
        IMap->output=ProfileParseKey(Value+8, &IMap->outmods);
    }
    else if (strncmp(Value, "exec:", 5)==0)
    {
        IMap->action=ACT_EXEC;
        IMap->target=UnQuoteStr(IMap->target, Value);
    }
    else if (strcmp(Value, "killwin")==0) IMap->action=ACT_WINKILL;
    else if (strcmp(Value, "closewin")==0) IMap->action=ACT_WINCLOSE;
    else if (strcmp(Value, "shadewin")==0) IMap->action=ACT_WINSHADE;
    else if (strcmp(Value, "stickwin")==0) IMap->action=ACT_WINSTICK;
    else if (strcmp(Value, "minwin")==0) IMap->action=ACT_WINHIDE;
    else if (strcmp(Value, "hidewin")==0) IMap->action=ACT_WINHIDE;
    else if (strcmp(Value, "fullscr")==0) IMap->action=ACT_WINFULLSCR;
    else if (strcmp(Value, "below")==0) IMap->action=ACT_WINLOWERED;
    else if (strcmp(Value, "above")==0) IMap->action=ACT_WINRAISED;
    else if (strcmp(Value, "wide")==0) IMap->action=ACT_WINMAX_X;
    else if (strcmp(Value, "tall")==0) IMap->action=ACT_WINMAX_Y;
    else if (strcmp(Value, "ungrab_pointer")==0) IMap->action=ACT_UNGRAB_POINTER;
    else if (strcmp(Value, "ungrab_mouse")==0) IMap->action=ACT_UNGRAB_POINTER;
    else if (strcmp(Value, "ungrab")==0) IMap->action=ACT_UNGRAB_POINTER;
    else if (strcmp(Value, "desktop:prev")==0) IMap->action=ACT_PREV_DESKTOP;
    else if (strcmp(Value, "desktop:next")==0) IMap->action=ACT_NEXT_DESKTOP;
    else if (strcmp(Value, "desktop:add")==0) IMap->action=ACT_ADD_DESKTOP;
    else if (strcmp(Value, "desktop:del")==0) IMap->action=ACT_DEL_DESKTOP;
    else if (strncmp(Value, "desktop:", 8)==0)
    {
        IMap->action=ACT_SWITCH_DESKTOP;
        IMap->target=CopyStr(IMap->target, Value + 8);
    }
    else IMap->output=ProfileParseKey(Value, &IMap->outmods);


    return(TRUE);
}


TProfile *ProfileGet(const char *Apps)
{
    ListNode *Node;
    TProfile *Profile;

    Node=ListFindNamedItem(Profiles, Apps);
    if (Node) return((TProfile *) Node->Item);

    Profile=(TProfile *) calloc(1, sizeof(TProfile));
    Profile->Events=(TInputMap *) calloc(255, sizeof(TInputMap));
    Profile->Apps=CopyStr(Profile->Apps, Apps);
    ListAddNamedItem(Profiles, Profile->Apps, Profile);
    return(Profile);
}

TProfile *ProfileParse(const char *RawConfig)
{
    TProfile *Profile;
    TInputMap *IMap;
    char *Name=NULL, *Value=NULL, *Config=NULL;
    const char *ptr;
    unsigned int in, out;

    if (! Profiles) Profiles=ListCreate();

    ptr=GetToken(RawConfig, "\\S", &Value, GETTOKEN_QUOTES);
    if (strcmp(Value, "all")==0) AddToAllProfiles=MCatStr(AddToAllProfiles, " ", ptr, NULL);
    else
    {

        Profile=ProfileGet(Value);
        if (Profile)
        {
            Config=ProfilePreProcess(Config, ptr);

            ptr=GetNameValuePair(Config, "\\S", "=", &Name, &Value);
            while (ptr)
            {
                IMap=&Profile->Events[Profile->NoOfEvents];
                if (ProfileParseInputMapping(IMap, Name, Value))
                {
                    Profile->NoOfEvents++;
                    ProfileKeyAddAlternatives(Profile, IMap);
                }

                ptr=GetNameValuePair(ptr, "\\S", "=", &Name, &Value);
            }

        }
    }

    Destroy(Name);
    Destroy(Value);
    Destroy(Config);

    return(Profile);
}



int ProfileMatchesApp(const char *Profile, const char *AppCmdLine)
{
    char *Tempstr=NULL, *Apps=NULL, *App=NULL;
    const char *ptr, *p_Args;
    int result=FALSE;

    p_Args=GetToken(Profile, "\\S", &Apps, GETTOKEN_QUOTES);

    ptr=GetToken(Apps, ",", &App, 0);
    while (ptr)
    {
        Tempstr=CopyStr(Tempstr, App);
        if (StrValid(p_Args)) Tempstr=MCatStr(Tempstr, " ", p_Args, NULL);
        if (fnmatch(Tempstr, AppCmdLine, FNM_CASEFOLD)==0)
        {
            result=TRUE;
            break;
        }

        ptr=GetToken(ptr, ",", &App, 0);
    }

    Destroy(Tempstr);
    Destroy(Apps);
    Destroy(App);

    return(result);
}


int ProfileLoad(const char *Path)
{
    STREAM *S;
    char *Tempstr=NULL;
    int RetVal=FALSE;

    S=STREAMOpen(Path, "r");
    if (S)
    {
        if (isatty(1)) printf("Loading config: %s\n", Path);
        RetVal=TRUE;
        Tempstr=STREAMReadLine(Tempstr, S);
        while (Tempstr)
        {
            StripTrailingWhitespace(Tempstr);
            StripLeadingWhitespace(Tempstr);

            if (StrValid(Tempstr) && (*Tempstr != '#')) ProfileParse(Tempstr);
            Tempstr=STREAMReadLine(Tempstr, S);
        }
        STREAMClose(S);
    }

    Destroy(Tempstr);

    return(RetVal);
}


void ProfilesClear()
{
    ListClear(Profiles, ProfileDestroy);

//we build a global list of ALL keygrabs, and listen to them all,
//then perform an action for the current window, or else just pass
//the keystroke through
    if (! Grabs) Grabs=(TProfile *) calloc(1, sizeof(TProfile));
    else Destroy(Grabs->Events);

//must do this if we've destroyed Grabs->Events!
    Grabs->NoOfEvents=0;

    AddToAllProfiles=CopyStr(AddToAllProfiles, "");
}



TProfile *ProfilesReload(const char *Paths)
{
    char *Tempstr=NULL, *Path=NULL;
    const char *ptr;
    glob_t Glob;
    struct stat Stat;
    int i, Loaded=FALSE;

    if (! Profiles) Profiles=ListCreate();
    ProfilesClear();

//Paths is a colon-separated list of directories to search in
    ptr=GetToken(Paths, ":", &Path, 0);
    while (ptr)
    {

// ~/ means 'current users home directory'
        if (strncmp(Path, "~/",2)==0)
        {
            Tempstr=MCopyStr(Tempstr, GetCurrUserHomeDir(), Path+1, NULL);
            Path=CopyStr(Path, Tempstr);
        }

        stat(Path, &Stat);
        if (S_ISDIR(Stat.st_mode))
        {
            //glob for config files and load them all
            Tempstr=MCopyStr(Tempstr, Path, "/*", 0);
            glob(Tempstr, 0, 0, &Glob);
            for (i=0; i < Glob.gl_pathc; i++)
            {
                if (ProfileLoad(Glob.gl_pathv[i])) Loaded=TRUE;
            }
            globfree(&Glob);
        }
        else ProfileLoad(Path);

        ptr=GetToken(ptr, ":", &Path, 0);
    }

    if (Loaded==FALSE) printf("ERROR: failed to load config '%s'\n", Path);

    Destroy(Tempstr);
    Destroy(Path);

    return(Grabs);
}



TProfile *ProfileForApp(const char *AppCmdLine)
{
    TProfile *Profile=NULL, *Default=NULL;;
    ListNode *Curr;

    Curr=ListGetNext(Profiles);
    while (Curr)
    {
        if (ProfileMatchesApp(Curr->Tag, AppCmdLine)) Profile=(TProfile *) Curr->Item;
        if (strcmp(Curr->Tag, "default")==0) Default=(TProfile *) Curr->Item;

        Curr=ListGetNext(Curr);
    }

    if (Profile) return(Profile);

    return(Default);
}




