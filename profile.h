#ifndef JP_PROFILE_H
#define JP_PROFILE_H

#include "common.h"

//less or more are exclusive, so use a single bit
#define ABS_LESS 0
#define ABS_MORE 1
#define FLAG_ACTIVE 4

typedef struct
{
char *Apps;
char *Devices;
int NoOfEvents;
TInputMap *Events;
} TProfile;

TProfile *ProfileParse(const char *Config);
TProfile *ProfileForApp(const char *Name);
void ProfilesReload(const char *Path);

#endif
