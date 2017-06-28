# Overview

Simfiles can control the motion of notes through modifiers.

Typically this is done through a list that tells the game what time to
activate each mod, how long it should run for, and sometimes how to tween it
on and off.

## Mods file

This is an example of the simplest possible mods file.  One mod that lasts
for four beats.

```lua
local mods= {
	{column= "all", start= 0, length= 4, 'some_mod_name'},
}

return mods
```

## Loading the mods file

Assuming the mods file is named "notefield_mods.lua" and placed in the song
folder, it can be loaded by having an actor like this among the other actors
in the simfile.

```lua
Def.Actor{
	OnCommand= function(self)
		local mods_filename= "notefield_mods.lua"
		local notefields= load_notefield_mods_file(GAMESTATE:GetCurrentSong():GetSongDir()..mods_filename)
	end,
}
```

load_notefield_mods_file will execute the file, find the notefield(s), and
apply the mods to the notefield(s).

load_notefield_mods_file returns a table containing the notefields it found.
Having handles for the notefields may be useful later.


# Mods table details

## Querying the system

### Targets list
The functions for getting a list of targets must be called on a NoteField or
NoteFieldColumn actor.

load_notefield_mods_file returned a table containing the notefields, so that
is used in this example.

```lua
local notefield_targets= notefields[PLAYER_1]:get_mod_target_info()
local column_targets= notefields[PLAYER_1]:get_columns()[1]:get_mod_target_info()
-- Check whether transform_rot_z exists
if notefield_targets.transform_rot_z then
	...
end
-- Print the list to the log file for reading.
foreach_ordered(notefield_targets, function(name, exists) do Trace(name) end)
```

### Custom mods list
```print_custom_mods_info()``` will print a list of all available custom mods
to Logs/log.txt.  The list will have all parameters, operators, and examples
for each mod.


## Some simple examples

Only affect column 1:
```lua
{column= 1, start= 0, length= 4, 'some_mod_name'},
```

Only affect columns 1 and 2:
```lua
{column= {1, 2}, start= 0, length= 4, 'some_mod_name'},
```

Use seconds instead of beats for all time fields in this mod entry:
```lua
{column= "all", time= 'second', start= 0, length= 4, 'some_mod_name'},
```

Intensify mod:
```lua
{column= "all", start= 0, length= 4, 'some_mod_name', 2},
```

Affect a different note attribute than the one the mod is normally for:
```lua
{column= "all", start= 0, length= 4, target= 'note_pos_y', 'some_mod_name'},
```

## Detailed description

Each modifier is actually an equation.

The ```2``` in the "Intensify mod" example is actually a parameter to change
one number in the equation.

Some equations can have more than one parameter.  To set more than one
parameter, you must use a table.

```lua
{column= "all", start= 0, length= 4, 'beat', {level= 2, time= 'music_second'}},
```

Normally, 'beat' takes effect on every beat.  Changing the time to use the
music second makes it take effect on every second.

The preceding example set the raw values for the parameters, replacing the
default values the equation would normally use.

Sometimes you want to just add to or multiply the default value by something.

```lua
{column= "all", start= 0, length= 4, 'beat', {level= {add= 2}, time= {mult= 2}}},
```

The param is set by a table describing how to add to or multiply the base
value.  The result is ```(default * mult) + add```.

Since the default of time is 'music_beat', ```time= {mult= 2}``` multiplies
the music beat by 2.

```lua
{column= "all", start= 0, length= 4, 'beat', {period= {value= 3, mult= 2, add= 4}}},
```

The result is ```(value * mult) + add```.


### Tweening parameters

```lua
{column= "all", start= 0, length= 4, on= 1, off= 1, 'beat', {level= 2}},
```

level will start at 0 and tween up to 2 in one beat.  During the last beat of
the mod, level will tween back to 0.

```lua
{column= "all", start= 0, length= 4, on= 1, off= 1, 'beat', {level= 2, period= 12}},
```

level and period will both tween.

period will tween from 6 (the default value in the mod info) to 12, then back
to 6.

```lua
{column= "all", start= 0, length= 4, on= 1, off= 1, 'beat', {level= 2, period= {value= 8, mult= 4, add= 2}}},
```

period.value will tween from 6 to 8, then back to 6.

period.mult will tween from 1 to 4, then back to 1.

period.add will tween from 0 to 2, then back to 0.


Only things that are set to raw numbers will tween.

```lua
{column= "all", start= 0, length= 4, on= 1, off= 1, 'beat', {level= 4, time= 'music_second'}},
```

level will tween, but time will not.

```lua
{column= "all", start= 0, length= 4, on= 1, off= 1, 'beat', {level= 4, time= {value= 'music_second', mult= 2}}},
```

level will tween.  time.mult will tween.  time.value will not.
