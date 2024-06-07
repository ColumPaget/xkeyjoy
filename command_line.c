#include "command_line.h"
#include "config.h"




int ParseCommandLine(int argc, char *argv[])
{
    CMDLINE *CmdLine;
    const char *p_arg;

    CmdLine=CommandLineParserCreate(argc, argv);

    p_arg=CommandLineFirst(CmdLine);
    if (p_arg)
    {
        if (strcmp(p_arg, "list")==0)
        {
            Config.Flags |= FLAG_LISTDEVS;
            p_arg=CommandLineNext(CmdLine);
        }
        else if (strcmp(p_arg, "mon")==0)
        {
            Config.Flags |= FLAG_MONITOR;
            p_arg=CommandLineFirst(CmdLine);
            if (! StrValid(p_arg))
            {
                printf("ERROR: no device-name given for monitor. Cannot continue\n");
                exit(1);
            }
            p_arg=CommandLineFirst(CmdLine);
        }
    }

    while (p_arg)
    {
        if (strcmp(p_arg, "-D")==0) Config.Flags |= FLAG_NODEMON;
        else if (strcmp(p_arg, "-debug")==0) Config.Flags |= FLAG_NODEMON;
        else if (strcmp(p_arg, "-c")==0) Config.ConfigPath=CopyStr(Config.ConfigPath, CommandLineNext(CmdLine));
        else if (strcmp(p_arg, "-n")==0) Config.Flags |= FLAG_NOEVDEV;
        else if (strcmp(p_arg, "-nodev")==0) Config.Flags |= FLAG_NOEVDEV;
        else if (strcmp(p_arg, "-t")==0) Config.DevTypes=CopyStr(Config.DevTypes, CommandLineNext(CmdLine));
        else if (strcmp(p_arg, "-devtypes")==0) Config.DevTypes=CopyStr(Config.DevTypes, CommandLineNext(CmdLine));
        else if (strcmp(p_arg, "-d")==0) Config.Devices=CopyStr(Config.Devices, CommandLineNext(CmdLine));
        else if (strcmp(p_arg, "-devs")==0) Config.Devices=CopyStr(Config.Devices, CommandLineNext(CmdLine));
        else if (strcmp(p_arg, "-v")==0) Config.Flags |= FLAG_VERSION;
        else if (strcmp(p_arg, "-X")==0) Config.Flags |= FLAG_XSENDEVENT;
        else if (strcmp(p_arg, "-T")==0) Config.Flags |= FLAG_XTEST;
        else if (strcmp(p_arg, "-version")==0) Config.Flags |= FLAG_VERSION;
        else if (strcmp(p_arg, "--version")==0) Config.Flags |= FLAG_VERSION;
        else if (strcmp(p_arg, "-h")==0) Config.Flags |= FLAG_HELP;
        else if (strcmp(p_arg, "-?")==0) Config.Flags |= FLAG_HELP;
        else if (strcmp(p_arg, "-help")==0) Config.Flags |= FLAG_HELP;
        else if (strcmp(p_arg, "--help")==0) Config.Flags |= FLAG_HELP;
        p_arg=CommandLineNext(CmdLine);

    }

    return(Config.Flags);
}


void DisplayHelp()
{
    printf("Usage:\n");
    printf("   xkeyjoy [options]\n");
    printf("   xkeyjoy list\n");
    printf("   xkeyjoy mon <device file>\n");
    printf("\n");
    printf("'xkeyjoy' without a modifer (list, mon) will run in service mode and process input from devices\n");
    printf("'xkeyjoy list' lists input devices that xkeyjoy can currently receive events from.\n");
    printf("'xkeyjoy mon' can be used to monitor events coming from a specific device. e.g. 'xkeyjoy mon event1'\n");
    printf("\nOptions for service mode are:\n");
    printf("-D                    don't background/daemonize, print debugging\n");
    printf("-debug                don't background/daemonize, print debugging\n");
    printf("-n                    don't use evdev, just use x11 key events\n");
    printf("-nodev                don't use evdev, just use x11 key events\n");
    printf("-devtypes <types>     list of evdev device types to use, default is \"!keyboard,!mouse,!trackpad,!touchscreen,!tablet\"\n");
    printf("-t <types>            list of evdev device types to use, default is \"!keyboard,!mouse,!trackpad,!touchscreen,!tablet\"\n");
    printf("-devs <list>          list of evdev devices to use, utilizes the full device name\n");
    printf("-d <list>             list of evdev devices to use, utilizes the full device name\n");
    printf("-c <path>             path to config file or directory containing config files\n");
    printf("-X                    use only XSendEvent for sending keystrokes (default is try XTest, then XSendEvent)\n");
    printf("-T                    use only XTest for sending keystrokes (default is try XTest, then XSendEvent)\n");
    printf("-v                    output version info\n");
    printf("-version              output version info\n");
    printf("--version             output version info\n");
    printf("-h                    this help\n");
    printf("-?                    this help\n");
    printf("-help                 this help\n");
    printf("--help                this help\n");
    printf("\n");
    printf("By default xkeyjoy runs equivalent to \"xkeyjoy -devtypes '!keyboard,!mouse,!trackpad,!touchscreen,!tablet'\". Events for keyboards, mice and trackpads are handled through X11 keygrabs rather than evdev. This is because grabbing these devices via evdev can cause problems for other applications. Thus by default xkeyjoy monitors only switches (lid switch etc) and 'game' devices (devices that have ABS controls, but are not pointer devices. These should be gamepads and joysticks).\n");
}

