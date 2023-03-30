#ifndef XKEYJOY_CONFIG_H
#define XKEYJOY_CONFIG_H

#include "common.h"


#define FLAG_HELP      1
#define FLAG_VERSION   2
#define FLAG_LISTDEVS  4
#define FLAG_MONITOR   8
#define FLAG_NODEMON  16
#define FLAG_DEBUG    32
#define FLAG_NOEVDEV  64


typedef struct
{
int Flags;
char *ConfigPath;
char *DevTypes;
char *Devices;
} TConfig;

extern TConfig Config;

void ConfigInit();

#endif
