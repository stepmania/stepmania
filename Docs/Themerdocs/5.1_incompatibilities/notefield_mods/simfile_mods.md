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

For those unfamiliar with documentation, the above text is known as an
example.  An example is code that can be put into a file and run, either
as-is, or with some minor change.  A minor change would be changing
'some_mod_name' to the name of an actual mod.

Any time you see an example, you should add it to a mods file and try it out.

Use the mods preview described in mods_preview.md to see mods while editing
steps without needing to use gameplay or the editor's play mode.


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
The list of current targets and what they do is in mod_targets.md.


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

Affect the notefield instead of a column:
```lua
{field= "all", start= 0, length= 4, target= 'transform_pos_y', 'some_mod_name'},
```

## Noteskins

### Fixed noteskins
```lua
local mods= {
	noteskin= "lambda",
}
```
Use the lambda noteskin instead of whatever the player picked.

```lua
local mods= {
	noteskin= {
		{name= "lambda", field= 1},
		{name= "exactV2", field= 2},
	},
}
```
Use lambda for player 1, exactV2 for player 2.

### Changing noteskins

```lua
local mods= {
	noteskin= {
		{name= "lambda"},
		{name= "exactV2"},
	},
	{column= {1, 2}, start= 0, length= 4, target= 'note_skin_id', 0},
	{column= {3, 4}, start= 0, length= 4, target= 'note_skin_id', 1},
	{column= {1, 4}, start= 0, length= 4, target= 'layer_skin_id', 0},
	{column= {2, 3}, start= 0, length= 4, target= 'layer_skin_id', 1},
}
```
Use lambda for the notes in columns 1 and 2, exactV2 for the notes in columns
3 and 4.

The receptors in columns 1 and 4 use lambda, while columns 2 and 3 use
exactV2 for the receptors.

### Random noteskins
```lua
local mods= {
	noteskin= {
		{random= 4}
	},
}
```
Pick 4 random noteskins to use.  Random will not pick noteskins the player
has hidden on the "Shown Noteskins" list.


## Detailed description

Each modifier is actually a named equation.

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

#### Input fields
```'music_second'``` is a name used to mean the current music second when the
equation is evaluated.

Other named input fields are in mod_inputs.md.


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

### Short name form parameters

These two examples are equivalent.
```lua
{column= "all", start= 0, length= 4, 'some_mod_name', {level= {value= 1, add= 2, mult= 3}}},
```

```lua
{column= "all", start= 0, length= 4, 'some_mod_name', {level= {v= 1, a= 2, m= 3}}},
```


### String form parameters

These three examples are equivalent.
```lua
{column= "all", start= 0, length= 4, 'some_mod_name', {level= {value= 1, add= 2, mult= 3}}},
```

```lua
{column= "all", start= 0, length= 4, 'some_mod_name', "v1 +2 *3"},
```

```lua
{column= "all", start= 0, length= 4, 'some_mod_name', "1 +2 *3"},
```

These two examples are equivalent.
```lua
{column= "all", start= 0, length= 4, 'some_mod_name', {level= {value= 1, add= 2, mult= 3}, period= {v= 4, m= 5, a= 6}}},
```

```lua
{column= "all", start= 0, length= 4, 'some_mod_name', {level= "1 *3 +2", period= "4 *5 +6"}}
```

The parts of the string must be separated by spaces.
They can be in any order.


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


Only level and things that are set to raw numbers will tween.

```lua
{column= "all", start= 0, length= 4, on= 1, off= 1, 'beat', {level= 4, time= 'music_second'}},
```

level will tween, but time will not.

```lua
{column= "all", start= 0, length= 4, on= 1, off= 1, 'beat', {level= 4, time= {value= 'music_second', mult= 2}}},
```

level will tween.  time.mult will tween.  time.value will not.


### Using equations for parameters

For more complex behavior, you can write an equation for a parameter.

```lua
{column= "all", start= 0, length= 4, on= 1, off= 1, 'beat', {level= {'*', 'y_offset', 1/64}}}
```

Instead of a raw number for level, this uses the y_offset of the note.
Multiplying it by 1/64 makes the y_offset smaller, to make its effect less
extreme.

The equation will tween because level is set by the equation.

```lua
{column= "all", start= 0, length= 4, on= 1, off= 1, 'beat', {period= {'*', 'y_offset', 1/64}}}
```

period will not tween because period is set to an equation, and period is not
level.

Defining what should or should not be tweened in more detail would be too
complex in the modifier data.


## Changing operators in a modifier

Some modifiers allow changing the operators used in parts of the equation.

```lua
{column= "all", start= 0, length= 4, 'drunk', {}, {wave= 'tan'}}
```

This example use tan instead of cos for the wave in drunk.
Note the params table is also absent, though blank in this example.

Operators cannot be equations and cannot be tweened, and must be valid
operator names.

```lua
local ops_list= ModValue.get_operator_list()
-- Check whether an operator exists.
if ops_list['*'] then
...
end
-- Print the names of all the operators that exist.
for name, exists in pairs(ops_list) do Trace(name) end
```


# Writing custom mods

It is possible to create custom modifiers and use their names in the mods
table.

A mods file with its own custom mods looks like this:

```lua
local custom_mods= {
	two= {
		target= 'note_pos_x',
		equation= function(params, ops)
			return {ops.level, params.level, params.other}
		end,
		params= {
			level= 1, other= 1,
		},
		ops= {
			level= '*',
		},
		examples= {
			{"multiply 2 by 4", "two, {level= 2, other= 4}"},
			{"divide 2 by 4", "two, {level= 2, other= 4}, {level= '/'}"},
		},
	},
}

local mods= {
	{column= "all", start= 0, length= 4, 'two'},
}

return mods, custom_mods
```

Note the both the mods table and the custom_mods table are returned by the
file.

## Custom mod entry parts

* target

If the mod line in the mods table does not set the target, this target will
be used.

* equation

The equation function must return a table written in the equation language.

The equation language is detailed in mod_language.md.

The params and ops arguments to the equation function are the parameters and
operators set by the entry in the mods table.  Default values from the custom
mod entry are used for things not given in the mods table.

* params

Default values for the params for the equation function.

* ops

Default operators for the operators for the equation function.

* examples

Examples of using the operators.  The two strings of each example will be
printed on different lines when print_custom_mods_info is called.


## Custom mod redirs

```lua
local custom_mods= {
	three= {
		redir= 'two',
		params= {
			other= {mult= 2},
		},
	},
}
```
```lua
{column= "all", start= 0, length= 4, 'three'}
```
is equivalent to
```lua
{column= "all", start= 0, length= 4, 'two', {other= {mult= 2}}}
```

Except params set inside the redir will not be tweened.


## Notes

### Naming

Name parameters and operators something that conveys purpose or intended use
or effect.

```level``` is a special parameter name.  The custom mods system has special
handling for it, and your custom mod should have a level param if it only has
one param.
```lua
{column= "all", start= 0, length= 4, 'two', 5}
```
From the equation function's perspective, params.level is set to 5.


```level``` has no special handling as an operator name.


### Sub Equation handling

When possible, allow things in params to be equations.  Don't use them
directly, just use them as operands in the equation table.  If you use a
param directly, then it won't be tweenable from the mods table.

```lua
equation= function(params, ops)
	return {'*', params.level * params.other, 2}
end,
```

Multiplying params.level and params.other like this will not work if they are
not raw numbers.  If the mods table entry using this modifier tries to tween
them, it won't work.

```lua
equation= function(params, ops)
	return {'*', params.level, params.other, 2}
end,
```

This will work, because they are used as equation operands, and the equation
multiplies them.



# Using an equation instead of a modifier name

```lua
{column= "all", start= 0, length= 4, target= "column_rot_z", {'-', 'music_beat', 'start_beat'}}
```

Rotate the column around the z axis by 1pi every beat after the mod starts.

The target must be specified.

There is no params or ops table for the equation.

The custom mods system works by converting the mod name and params to an
equation like this.

If on or off tween times are specified, the result of the equation will tween
from 0 to full during the on segment, then from full to 0 during the off
segment.
