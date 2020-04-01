SUMMARY
=======

xkeyjoy is a daemon process that maps input from gamepads via evdev or from mice and keyboards via X11. These inputs can be used to launch programs or be mapped to keypresses to control running programs. xkeyjoy can detect which window has current focus, hopefully can figure out which application is running, and can use a unique configuration for that application. Currently there is no GUI, all configuration is done via config files.

For xkeyjoy to detect which application has the current focus that application must either expose it's process id, or its commandline as X11 properties. xkeyjoy first looks for the `_NET_WM_PID` property that contains the process id, and if it gets that it looks the process up in /proc to obtain its full command-line. If the application doesn't expose the `_NEW_WM_PID` property, then xkeyjoy checks for the `XA_WM_COMMAND` property that should contain the applications full command-line. If that's missing, then xkeyjoy falls back to the `XA_WM_NAME` property, from which it can at least obtain the program's name. It will then try to match the program's command-line, or name if that's all it's got, against configuration profiles to select the right one.


DISCLAIMER
==========

This is free software. It comes with no guarentees and I take no responsiblity if it makes your computer explode or opens a portal to the demon dimensions, or does anything. It is released under the Gnu Public Licence version 3.


INVOCATION
==========

xkeyjoy <options>

*  -d           don't daemonize (don't fork into background)
*  -c <path>    path to a config file, or a directory containing config files
*  -v           output program version
*  -version     output program version
*  --version    output program version
*  -?           output program help
*  -h           output program help
*  -help        output program help
*  --help       output program help

Normally you'll just run ./xkeyjoy to fork it into the background. By default xkeyjoy looks for config files in the '/etc/xkeyjoy' and '~/.xkeyjoy' directories.


CONFIG FILES
============

By default config files live in the '/etc/xkeyjoy' directory for system-wide config files, or the '~/.xkeyjoy' directory. xkeyjoy will try to load any files in these directories as config files, building its config from the result of merging all of them. Config files can have any name. A default config file is shipped with the source under the name 'profiles.conf'.

Each line in a config file is a config for one application, and has the form:

```
<app command line> <mapping>...
```

Each mapping has the form:

```
<map type>=<map action>
```

A 'map type' describes an event that xkeyjoy will respond to. These can be:

```
btn:<name>       a gamepad (or other device) button reported through evdev.
sw:<name>        a switch (lid-switch etc) reported through evdev.
abs<num>         a joystick axis reported through evdev.
xbtn:<name>      a button event reported through X11 (these will normally be mouse buttons)
xkb:<name>       a keyboard event reported through X11
```

A 'map action' is an action carried out in response to an event. These can be:

```
<keyname>          a keypress
exec:<progname>    a program to launch
```


For example:

```
xgalaga btn:X=space btn:B=space btn:Y=space btn:A=a btn:left=left btn:right=right btn:start=k
```

This configuration applies to the application 'xgalaga' and maps buttons on a gamepad to keys on the keyboard. This example only uses the program's name, but the entire command-line can be matched by putting the command-line string in single quotes, like this:

```
'dosbox *CARNAGE.EXE*' btn:X=lctrl btn:B=lshift btn:Y=lshift btn:A=space btn:ltrig=lctrl btn:rtrig=lshift btn:left=left btn:right=right btn:up=up btn:down=down btn:start=space
```

This allows us to match the full command-line 'dosbox CARNAGE.EXE' to provide gamepad mappings for a particular game that runs under dosbox. Notice that the command-line match can use shell-style pattern matching. 


There are two special 'command-line' values:

* 'default' 
	Applies when no other matching config can be found.
* 'all' 
	Is added to all profiles.
	

So, for example,

```
default xkb:calc=exec:xterm xkb:www=exec:otter-browser
```

Will cause the calculator and www buttons found on many internet keyboards to launch xterm or the otter webbrowser respectively.

However,

```
all xkb:calc=exec:xterm xkb:www=exec:otter-browser
```

will cause these keygrabs to be added to all profiles defined in the config file.

Finally,

```
all xkb:calc=exec:xterm xkb:www=exec:otter-browser
default 
```

will cause the keygrabs to be added to all profiles, including the default one (no other keys or buttons are mapped in 'default', but they could be). This means these keygrabs should be active for all windows, whether there's a profile for them or not.


Joystick input is matched with the 'abs[n]' form of map type. This allows a joystick axis to be mapped to a keypress if it goes over a certain value. For example:

```
XGalaga++ btn:X=space btn:B=space btn:Y=space btn:A=space btn:tl=space btn:tr=space btn:left=left btn:right=right btn:start=s abs0<-4000=left abs0>4000=right abs3<-4000=left abs3>4000=right
```

This uses axis '0' (abs0) and axis '3' (abs3) to trigger the left and right arrow keys if the value of the axis is less than -4000 or greater than +4000. 




INPUTS
======

Inputs recognized by xkeyjoy are:

```
Gamepad Buttons: btn:A, btn:B, btn:C, btn:X, btn:Y, btn:Z, btn:select, btn:start, btn:ltrig, btn:rtrig
Gamepad DPad:    btn:up, btn:down, btn:left, btn:right
Gamepad Sticks:  abs0, abs1, abs2, abs3
Switches:        sw:lid, sw:rfkill, sw:dock, sw:tablet
X11 keypresses:  xkb:<keyname>
```

Gamepad sticks are configured differently than other input types. Firstly they are divided into axes, so abs0 and abs1 will usually be the x and y axis of the first stick, and abs2 and abs3 will be the x and y axes of the second stick. These axes are multi-valued inputs depending on how far they've been moved, so to map these to a keyboard input we have to specify a maximum or minimum value that will trigger the keysend. This is written like:

```
abs0<-4000=left abs0>4000=right
```

Xkeyboard events can be specified using the syntax `xkb:<keyname>`, e.g. `xkb:home`, `xkb:pause`, `xkb:F1`. It's normally most useful to map these to 'exec' actions.  

There are only a few switch events, only two of which are very useful: the lid switch `sw:lid` and the rfkill switch `sw:rfkill`. 

You can map the lid switch activating to screen lock with:

```
	sw:lid=xautolock\ -locknow
```

The other switches are 'inserted into dock' `sw:dock`, 'tablet mode' `sw:tablet`, 'headphones inserted' `sw:headphone` `sw:phone`, 'microphone inserted `sw:microphone` `sw:mic`, 'line-out inserted' `sw:lineout`, 'lin-in inserted' `sw:linein`.

All switches have an 'off' version, written like `sw:lid-off` or `sw:microphone-off` which triggers on the release/deactivate event rather than the activated event for the switch.


EXEC ACTION
===========

the `exec:` action launches the specified command-line.  To launch command-lines that contain spaces you need to use '\' to quote the spaces, like so:

```
	xkb:search=exec:firefox\ www.google.com xkb:shop=exec:firefox\ www.ebay.com
```



KEYNAMES
========

Most of the keynames that xkeyjoy recognizes either to send or as keygrab inputs, are simply the character that is sent when the key is pressed. However, there's a number of keys that don't send a character by default. These are mostly 'action' keys like the function keys and the media control keys and internet keys found on some keyboards. Recognized keynames are:

Function keys: F1 F2 F3 F3 F5 F6 F7 F8 F9 F10 F11 F12 F13 F13
Action keys: esc, enter, open, close, pause, copy, cut, paste, clear, search, sleep, standby, lshift, rshift, lctrl, rctrl, win, menu
Arrow keys: left, right, up, down
Keypad keys: home, end, insert, delete pageup, pagedown
System keys: scrlck, pause, print
Internet keys: www, shop, media, faves, back, forward, homepage, mail, messenger,  eject, calculator, lightbulb, wlan, webcam
Media keys: play, mute, volup, voldown

'esc' is the standard escape key, 'win' is the windows key, 'volup' and 'voldown' are the increase and decrease volume keys on media keyboards. 'play' is the 'play or pause' key on media keyboards. 'scrlck' is the scroll-lock key, 'print' is the print-screen key and 'pause' is the pause/break key.


SUPPORTED GAMEPADS
==================

So far PS4, Logitech and 8bitdo gamepads have been seen to work with xkeyjoy. Some other gamepads do not send the appropriate keycodes, and so do not work at current.
