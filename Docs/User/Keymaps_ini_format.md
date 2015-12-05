# Overview

The Keymaps.ini file tells StepMania how the keys on the keyboard or other
input devices are mapped.  Other input devices are referred to as joysticks
for convenience.

You should only edit the Keymaps.ini file directly if some malfunction
prevents mapping keys through the Config Key/Joy Mappings screen.


## Location
Windows: %APPDATA%/StepMania 5.0/Save/Keymaps.ini  
Linux: ~/.stepmania-5.0/Save/Keymaps.ini  
OS X: ~/Library/Preferences/StepMania 5/Save/Keymaps.ini  
(OS X location may be incorrect, report it if you use OS X and that location
is incorrect)


## Structure
An ini file is split up into sections.  Each section begins with the section
name in square brackets like this:
```
[dance]
```
In the Keymaps.ini file, each section contains the button mappings for that
game type.  Only sections for game types you have played will be in the file.
The beginning of a section marks the end of the previous section.

A section contains a list of key value pairs.  Everything before the equals
sign is the key, and everything after the equals sign is the value.
```
2_Start=Joy1_B8:Key_space
```
In this example, ```2_Start``` is the key, and ```Joy1_B8:Key_space``` is
the value.

StepMania uses the key to store which player the button is for, and the name
of the button.  ```2_Start``` is the Start button for Player 2.

The value is a colon separated list of keyboard or joystick keys that map to
the button.  There can only be two entries in the list.

In the example ```Joy1_B8:Key_space```, ```Joy1_B8``` is the first joystick
key mapped, and ```Key_space``` is the second.  The first part of the key is
the name of the device, and the second part is the name of the button.

The keyboard always has the device name Key. Other input devices are all
named Joy, with a number for the order they were detected in.  Buttons on a
joystick are numbered from 1 to however many the joystick has.  There is a
section at the end for the names of keyboard keys.

You can use the Test Input screen to discover the names of keys or buttons.
It is in the Input Options section of the service menu.
If you have a malfunctioning device that is preventing using menus, or a
broken keymap, you can reset to the default mapping by holding F3 and
pressing P.  You should find the Automap Joysticks option in the Advanced
Input Options screen and turn it off so that your custom mapping doesn't get
overwritten when StepMania sees different devices connected.


# Example

Let's imagine that you want to map the E key to Player 1 Left in the dance
game mode.
* Find the dance game section.  It starts with ```[dance]```.
* Find the entry for Player 1 Left.  Player 1 means it will start with 1, and
the button name is Left, so the key name for the entry will be ```1_Left```.
* Change its value to the E key.  The keyboard device name is Key, and the
button name is e, so the full name is ```Key_e```.
* Finished example:
```
1_Left=Key_e
```


### Non-printable keys
If the key normally types a single character when pressed, that single
character is considered its printable character.

Some keys cannot be represented by a single character, so the name of the key
is used instead.

The following keys use their names instead of a single character:

* space
* backslash
* comma
* period
* colon
* pgdn
* pgup
* left shift
* right shift
* enter
* tab
* caps lock
* left ctrl
* right ctrl
* left alt
* right alt
* delete
* insert
* end
* home

This list may be incomplete.
