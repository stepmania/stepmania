This is mostly a half-organized list of things that need to be covered.


```lua
local mods= {
	columns= 4,
	{column= 1, start= 4, length= 4, on= 1, off= 2, 'drunk', {level= 2}, {wave= 'tan'}},
}

return mods
```

```on= 1``` sets this mod to spend one beat tweening on.  level will start at 0 and grow to 2.

```off= 2``` means this mod will tween off in two beats.  level goes from 2 to 0.

```'drunk'``` is the name of a custom mod.

The table after it is parameters for that mod.

Second table is operators for the mod to use instead of its normal ones.

```print_custom_mods_info()``` will print a list of all available custom mods
to Logs/log.txt.  The list will have all parameters, operators, and examples
for each mod.

Custom mods in _fallback are under "Engine custom mods:".
Custom mods added by the theme are under "Theme custom mods:".  Custom mods
added by the simfile are under "Simfile custom mods:".

```set_theme_custom_mods(mods)``` can be used by the theme to add more in
addition to the ones in _fallback.

```set_simfile_custom_mods(mods)``` can be used by the simfile to add more.

If a custom mod added by the theme has the same name as one in _fallback, it
overrides the one in _fallback.

A custom mod from the simfile with the same name overrides the others.

```get_custom_mods_list()``` returns a table for checking whether a mod
exists.
```lua
local all_customs= get_custom_mods_list()
if all_customs.engine.drunk then
...
end
if all_customs.theme["( ͡° ͜ʖ ͡°)"] then
...
end
if all_customs.simfile["(╯°□°)╯︵ ┻━┻"] then
...
end
```

Mods and custom mods can be put in a separate file from actors.

```lua
local custom_mods= {
	one= {
		target= 'note_pos_x',
		equation= function(params, ops)
			return {ops.level, params.level}
		end,
		params= {
			level= 1,
		},
		ops= {
			level= '*',
		},
		examples= {
			{"normal", "'one'"},
			{"double", "'one', {level= 2}"},
			{"log", "'one', {level= 10}, {level= 'log'}"},
		},
	},
	two= {
		target= 'note_pos_x',
		equation= function(params, ops)
			return {ops.level, params.level, params.l2}
		end,
		params= {
			level= 1, l2= 1,
		},
		ops= {
			level= '*',
		},
		examples= {
			{"normal", "'two'"},
			{"quadruple", "'two', {level= 2, l2= 2}"},
			{"powerful", "'two', {level= 2, l2= 10}, {level= '^'}"},
		},
	},
	three= {
		target= 'note_pos_x',
		equation= function(params, ops)
			return {ops.level, params.level, params.l2, params.l3}
		end,
		params= {
			level= 1, l2= 1, l3= 1,
		},
		ops= {
			level= '*',
		},
	},
}

local mods= {
	columns= 4,
	{column= 1, start= 4, length= 4, 'drunk', {level= 2}, {wave= 'tan'}},
}

return mods, custom_mods
```

```load_notefield_mods_file(path)``` will load a file like that, add the
custom mods to the environment, and apply the mods to the player notefields.


Parts of a custom mod entry:

target controls the attribute that the mod sets by default.

equation is the mod language equation, which takes parameters and operators
to fill in equation parts that can change.

params is a table of parameters with default values.

ops is a table of operators with default values.

examples is a list of examples of using the custom mod.  Each entry in the
list is a short description and example code.  print_custom_mods_info puts
the description and the example code on separate lines.

Parameters should not be used directly, they should just be put into the
equation table.  That allows them to be equations.
```lua
		equation= function(params, ops)
			return {ops.level, params.level + params.l2, params.l3}
		end,
```
Adding level and l2 like that will break on and off tweening, because
tweening replaces level with an equation.

The parameter named "level" is special.  The on and off tween fields will
modify it to go from 0 to full when tweening on, and from full to 0 when
tweening off.


Tweenable mod params

```lua
	{column= "all", start= 4, length= 4, 'three', {level= 4, l2= {4, on= 1, off= 1, time= 'second'}, l3= 4}},
```
l2 will be tweened using the on and off times in its table.  If l2.time were blank, it would default to 'beat'.
