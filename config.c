#include "config.h"

TConfig Config;

void ConfigInit()
{
    memset(&Config, 0, sizeof(TConfig));

    Config.ConfigPath=CopyStr(Config.ConfigPath, "/etc/xkeyjoy:~/.xkeyjoy:~/.config/xkeyjoy");
    Config.DevTypes=CopyStr(Config.DevTypes, "!keyboard,!mouse,!trackpad,!tablet,!touchscreen");
    Config.Devices=CopyStr(Config.Devices, "");
}
