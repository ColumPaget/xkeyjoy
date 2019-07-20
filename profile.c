#include <linux/input.h>
#include "profile.h"
#include "proc.h"
#include <glob.h>

ListNode *Profiles=NULL;
char *AddToAllProfiles=NULL;

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


static void ProfileParseXKBInput(TInputMap *IMap, const char *Def)
{

	IMap->intype=EV_XKB;
	IMap->input=ProfileParseKey(Def+4, &IMap->inmods);
}

static void ProfileParseXButtonInput(TInputMap *IMap, const char *Def)
{
	IMap->intype=EV_XBTN;
	IMap->input=ProfileParseKey(Def, NULL);
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
Tempstr=MCopyStr(Tempstr, Config, " ", AddToAllProfiles, NULL);
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
const char *ptr;

		if ( strncmp(Name, "abs", 3)==0) ProfileParseAbsInput(IMap, Name);
		else if (strncmp(Name, "btn:", 4)==0) ProfileParseButtonInput(IMap, Name);
		else if (strncmp(Name, "xkb:", 4)==0) ProfileParseXKBInput(IMap, Name);
		else if (strncmp(Name, "xbtn:", 5)==0) ProfileParseXButtonInput(IMap, Name);
		else return(FALSE);

		if (strncmp(Value, "rootwin:", 8)==0) 
		{
			IMap->target=CopyStr(IMap->target, "root");
			ptr=Value + 8;
		}
		else if (strncmp(Value, "exec:", 5)==0) 
		{
			IMap->target=UnQuoteStr(IMap->target, Value);
			ptr=NULL;
		}
		else ptr=Value;

		IMap->output=ProfileParseKey(ptr, &IMap->outmods);

		return(TRUE);
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
if (strcmp(Value, "all")==0) AddToAllProfiles=CopyStr(AddToAllProfiles, ptr);
else
{
	Profile=(TProfile *) calloc(1, sizeof(TProfile));
	Profile->Events=(TInputMap *) calloc(255, sizeof(TInputMap));
	Profile->Apps=CopyStr(Profile->Apps, Value);
	
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
	
	ListAddNamedItem(Profiles, Profile->Apps, Profile);
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
	if (fnmatch(Tempstr, AppCmdLine, 0)==0)
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


void ProfileLoad(const char *Path)
{
STREAM *S;
char *Tempstr=NULL;

S=STREAMOpen(Path, "r");
if (S)
{
	Tempstr=STREAMReadLine(Tempstr, S);
	while (Tempstr)
	{
		StripTrailingWhitespace(Tempstr);
		StripLeadingWhitespace(Tempstr);
		if (StrValid(Tempstr)) ProfileParse(Tempstr);
		Tempstr=STREAMReadLine(Tempstr, S);
	}
	STREAMClose(S);
}
else if (isatty(1)) printf("ERROR: failed to load config '%s'\n", Path);

Destroy(Tempstr);
}


void ProfilesReload(const char *Dirs)
{
char *Tempstr=NULL, *Dir=NULL;
const char *ptr;
glob_t Glob;
int i;

if (! Profiles) Profiles=ListCreate();
ListClear(Profiles, ProfileDestroy);

//Dirs is a colon-separated list of directories to search in
ptr=GetToken(Dirs, ":", &Dir, 0);
while (ptr)
{

// ~/ means 'current users home directory'
if (strncmp(Dir, "~/",2)==0)
{
Tempstr=MCopyStr(Tempstr, GetCurrUserHomeDir(), Dir+1, NULL);
Dir=CopyStr(Dir, Tempstr);
}

//glob for config files and load them all
Tempstr=MCopyStr(Tempstr, Dir, "/*", 0);
glob(Tempstr, 0, 0, &Glob);
for (i=0; i < Glob.gl_pathc; i++) ProfileLoad(Glob.gl_pathv[i]);
globfree(&Glob);
ptr=GetToken(ptr, ":", &Dir, 0);
}

Destroy(Tempstr);
Destroy(Dir);
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




