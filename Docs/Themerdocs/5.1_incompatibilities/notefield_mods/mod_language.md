# Overview

The notefield modifier system is a miniature language for directly
controlling how the notes move, what size they are, their rotation, and all
other things related to rendering notes.

The language is composed of equations which result in a value when evaluated.
Equations are constructed of nested operators and operands.

Each moddable aspect of rendering a note has equations for determining its
value at any given time.  When the notefield needs to render a note, it runs
the equations for the parts and then renders the note at the resulting
position, rotation, size, and so forth.

Everything is guaranteed to be synced to the music because the current time
in the song forms the basis of the values that are put into the equations.

Effectively, these equations allow lua authors to compose new modifiers that
previously would need to be added to the engine.

# Mods table

This is a mods table that uses equations instead of the named mods described
in simfile_mods.md.  The fields in each mod entry are the same.

```lua
{
	{
		column= 1, -- This mod entry only affects column 1.
		target= 'column_pos_x', -- The column's x position will change.
		start= 8, -- Starting on beat 8.
		length= 4, -- Lasting for 4 beats.
		time= 'beat', -- start and length are beats.
		{'*',
		 {'-', 'music_beat', 'start_beat'} -- Subtract the start beat of this mod entry from the current music beat.
		 64 -- And multiply by 64.
		},
		-- The result of this equation is added to the already existing x value.
	},
	{
		column= "all", -- This mod entry affects all columns.
		target= 'column_zoom_y', -- The column's x position will change.
		start= 4, -- Starting on second 4.
		length= 4, -- Lasting for 4 seconds.
		time= 'second', -- start and length are seconds.
			.5, -- The simple value .5 instead of an equation.
		sum_type= '*', -- Multiply the existing zoom value by the value or equation result.
	},
	{
		field= 1, -- This affects the notefield instead of a single column.
		target= 'transform_rot_z', -- Rotate the notefield around z.
		start= 12, length= 4,
		-- If there is no time field, it defaults to beats.
		{'*', math.pi, {'-', 'music_beat', 'start_beat'}}, -- Rotations are in radians.  Multiply by pi to rotate a half circle every beat.
	}
}
```

The table tells the system how many columns and fields it effects.

Each entry specifies what column or field it is for, the attribute to change,
start time and length, an equation, and how the equation is combined with the
existing value.

## Mod entry fields

### column
This can be a number, or the string ```'all'```.  If it is a number, only that column is affected.  If it is ```'all'```, the equation is applied to all columns.

column must be nil for mods that target the field.

### field
The id of the field to affect, or ```'all'``` to affect all fields.

Using ```'all'``` or multiple fields affects the structure returned by
organize_notefield_mods_by_target, which has its own section.

### target
A single specific attribute that is affected.

If you have a NoteField or NoteFieldColumn actor, the get_mod_target_info
function of that actor will return a list of target names.

The target info table is index by target name.  Each valid target has a true value.  This way you can check for the existence of a target with ```if target_info.quantization_offset then```.

### start, length, and time
The mod's equation is applied when time >= start and time < start+length.

The equation does *not* apply when time == start+length because a mod that
starts at beat 1 and lasts for 1 beat is not expected to briefly overlap with
a mod that starts at beat 2.

If time is ```'second'```, then start and length are seconds instead of
beats.  This matters if you need a mod that starts during a stop.

time defaults to ```'beat'```, or you can explicitly put ```time= 'beat'```
in the mod entry.

### sum_type
Each target has a base value and multiple equations.  sum_type controls how
the result of this equation is combined with the result of previous
equations.

The sum_type can be one of a few operators:
```'+'```  The default, add the result of this equation to the existing value.
```'-'```  Subtract this result from the value, and set the result as the new
value.
```'*'```  Multiply the value by this result, and set the result as the new
value.
```'/'```  Divide the value by this result, and set the result as the new
value.
```'replace'```  Set the result of this equation as the new value.

### priority
Priority affects the order equations are applied in.  Priority can be any
integer, lower values mean sooner.  Priority 0 mods are evaluated before
priority 1 mods.  This defaults to 0.

```priority= -1, sum_type= 'replace', 0``` can be used to create a mod that
effectively sets the base value of a target to zero for its duration, then
other mods act on top of that base value.


## Target specifics

The list of current targets and what they do is in mod_targets.md.


# Mod equation language specifics

An expression in the mod language is a lua table.  The first element of the
table is the operator and the other elements are its operands.

Operands are the values a function operates on.  It's another word for
arguments or parameters.

With the exception of the phase operator, all operands can be expressions in
their own right.  The result of the sub expression then becomes the value the
parent operator uses.

Line breaks are added in this example in a vain attempt to make it easier to
read.  How to format equations for readability is up to you.
```lua
{'+', -- Add the operands together.
 start_mag, -- start_mag is some lua variable, pretend it's 5 if you want.
 {'*', -- Multiply the operands together.
	{'-', end_mag, start_mag}, -- Subtract start_mag from end_mag.
	{'/', -- Divide the first operand by the second.
	 {'-', 'music_beat', 'start_beat'},
	 'length_beats',
	}, -- (music_beat - start_beat) / length_beats
 }, -- (end_mag - start_mag) * divide_result
} -- start_mag + multiply_result
```

With this structure, you do not need to remember operator precedence rules
like "multiplication happens before addition" because the precedence is
explicit in the table structure.  Not needing to memorize precedence matters
when unfamiliar operators such as mod, spline, min, max, and so forth are
added.

Common operators have two names, a short single character name, and a full out name.

Some operators only have one operand and will emit an error if there is more
than one.


## Operand specifics

An operand can be a number, the name of some input field, or a table which is
an expression.

These are all valid operands:

```lua
-- The number 5.
5
```
```lua
-- The current music beat.
'music_beat'
```
```lua
-- The result of adding 1 and 2.
{'+', 1, 2}
```


## Input field names

Input field names are described in mod_inputs.md.

## Operator specifics

#### Fetching the operator list
This document might be outdated, or you may want to check whether a recently
added operator exists before using it.  The engine provides functions for
seeing what operators are available.

```ModValue.get_operator_list()``` returns a name indexed list of
operators.  ```ModValue.get_operator_list()['spline']``` will only be
true if the spline operator exists.

Put the table returned by get_operator_list in a variable if you need to
use it more than once.



### add
Names: ```'+'```, ```'add'```

Sums its operands, can have any number of operands.

```lua
-- equivalent to 1+2+3+4+5
{'+', 1, 2, 3, 4, 5}
```

### subtract
Names: ```'-'```, ```'subtract'```

Subtracts its operands, can have any number of operands.

```lua
-- equivalent to 1-2-3-4-5
{'-', 1, 2, 3, 4, 5}
```

### multiply
Names: ```'*'```, ```'multiply'```

Multiplies its operands, can have any number of operands.

```lua
-- equivalent to 1*2*3*4*5
{'*', 1, 2, 3, 4, 5}
```

### divide
Names: ```'/'```, ```'divide'```

Current result starts as the first operand.  This is then divided by each
operand.  There can be any number of operands.

Dividing by zero results in zero.  Intentionally, so that the second operand
can be the result of a sine function without giving infinity for a brief
moment.

```lua
-- equivalent to 1/2
{'/', 1, 2}
-- equivalent to (1/2)/3
{'/', 1, 2, 3}
```

### exp
Names: ```'^'```, ```'exp'```

Raises the first operand to the power of the second.  Can have extra operands
for higher exponentiation (why?!).

```lua
-- equivalent to 2^5
{'^', 2, 5}
-- equivalent to (2^5)^4 (2^20)
{'^', 2, 5, 4}
```

### log
Names: ```'v'```, ```'log'```

'v' is the logical visual inverse of '^'.  And log is the inverse of exp.

C's log function returns the natural log of a number.  Dividing the natural
log of a number by the natural log of some desired base results in the log in
that base.  So "log(2) / log(10)" results in the log base 10 of 2.

This log operator has that behavior.

```lua
-- equivalent to log(2) / log(10)
{'log', 2, 10}
-- equivalent to log(2)
{'log', 2}
-- equivalent to (log(2) / log(10)) / log(5)
{'log', 2, 10, 5}
```

### min
Names: ```'min'```

Returns the lowest of its operands.

```lua
-- result is -2
{'min', -2, 0, 1}
```

### max
Names: ```'max'```

Returns the highest of its operands.

```lua
-- result is 1
{'max', -2, 0, 1}
```

### mod
Names: ```'%'```, ```'mod'```

The modulus operator.  Modulus by zero results in zero.
Only two operands allowed.

```lua
-- result is 2
{'%', 5, 3}
```

### round
Names: ```'o'```, ```'round'```

'o' is round, thus, it is the rounding operator.

Rounds its first operand to the nearest multiple of the second.
Rounding to the nearest multiple of zero results in the first operand.
Only two operands allowed.

```lua
-- result is 6
{'round', 5, 3}
-- result is 5
{'round', 5, 0}
```

### floor
Names: ```'_'```, ```'floor'```

Rounds its first operator down (towards negative infinity) to the next
multiple of the second.
Only two operands allowed.

```lua
-- result is 3
{'floor', 5, 3}
-- result is 5
{'floor', 5, 0}
```

### ceil
Names: ```'ceil'```

No short name because it was getting silly.
Rounds its first operator up (towards positive infinity) to the next
multiple of the second.
Only two operands allowed.

```lua
-- result is 6
{'ceil', 5, 3}
-- result is 5
{'ceil', 5, 0}
```

For floor, ceil, and round, passing zero as the second operand results in the
first operand so that a sine or other wave can safely be the second operand.

### sin
Names: ```'sin'```

Really tempted to use '~' as the short name, because it's wavy.

Returns the sine of its operand.  The operand is multiplied by pi internally,
because it's radians and you were going to multiply it by pi anyway.

```lua
-- result is sin(pi), which is zero
{'sin', 1}
-- result is 0.7071
{'sin', .25}
```

### cos
Names: ```'cos'```

Returns the cosine of its operand.  The operand is multiplied by pi
internally, because it's radians and you were going to multiply it by pi
anyway.

```lua
-- result is cos(pi), which is -1
{'cos', 1}
```

### tan
Names: ```'tan'```

Returns the tangent of its operand.  The operand is multiplied by pi
internally, because it's radians and you were going to multiply it by pi
anyway.

```lua
-- result is tan(pi), which is close enough to zero
{'tan', 1}
```

### square
Names: ```'square'```

A square wave is similar to a sine wave, except it is only ever 1 or -1.  The
center of its peak is pi/2 (the same as sine), and the center of its trough
is 3pi/2 (the same as sine).

```lua
-- result is -1
{'square', 1}
```

### triangle
Names: ```'triangle'```

An idol group composed of Kanon, Junon, and Pinon, who are all Non,
duplicated with hologrammation technology.  Non thinks she can beat her
sister as an idol group by playing three parts at once, but it doesn't work
out.

The peak and trough centers are in the same place as sine's peak and trough.
This is a straight line from trough to peak where sine has a curved line.

```lua
-- result is 0
{'triangle', 1}
-- result is .5
{'triangle', .25}
```

For all wave functions (sin, cos, tan, square, triangle), the angle given is multiplied by pi internally because all math functions work in radians, which
would mean multiplying by pi anyway.  1 is half a circle, .5 is one quarter of a circle, and so forth.


### random
Names: ```'random'```

Returns a psuedo random value based on its operand.  This will be the same as
long its operand is the same, but will be different each time the song is
played.

```lua
-- Different result for every eval_beat, but the same result every frame for
-- a given eval_beat, but different each time the song is played.
{'random', 'eval_beat'}
```

### repeat
Names: ```'repeat'```

Uses modulus and addition to put its first operand into the range defined by
its second and third operands.  Effectively, if the first operand is some
value that continuously increases, the result of using repeat will be
repeating the given range.

```lua
-- Make music_beat repeat the range from 1 to 2 over and over.
{'repeat', 'music_beat', 1, 2}
```

### phase
Names: ```'phase'```

Intended for things that need to go through multiple different phases.

In each phase, the first operand is shifted by the phase start, multiplied by
the phase multiplier, then added to the phase offset to obtain the result.

The phases are passed in the second operand.  There is a default phase for
when the current value of the first operand does not fit into any phase.

Each phase is four numbers, which cannot be operands (or sub expressions or
equations).

The first number is the start of the phase.

The second number is the end of the phase.

The third number is the multiplier for the phase.

The fourth number is the offset for the phase.

If the first operand is between the start and end of a phase
(phase_start <= first_operand < phase_end), that phase is used for
calculating the result.

The result is ((first_operand - phase_start) * phase_multiplier) + phase_offset.

It's similar to a condition, but less flexible.

```lua
-- Uses the phase that starts at 1 and ends at 2, so the result is ((1-1) * 3) + 4, which is 4.
{'phase', 1, {default= {0, 0, 1, 0}, {0, 1, 2, 3}, {1, 2, 3, 4}, {2, 3, 4, 5}}}
-- Uses the phase that starts at 0 and ends at 1, so the result is ((.5-0) * 2) + 3, which is 4.
{'phase', .5, {default= {0, 0, 1, 0}, {0, 1, 2, 3}, {1, 2, 3, 4}, {2, 3, 4, 5}}}
-- Uses the phase that starts at 2 and ends at 3, so the result is ((2.5-2) * 4) + 5, which is 7
{'phase', 2.5, {default= {0, 0, 1, 0}, {0, 1, 2, 3}, {1, 2, 3, 4}, {2, 3, 4, 5}}}
-- 3.5 is not in the range of any of the phases, the default phase is used.  Result is ((3.5-0) * 1) + 0, which is 3.5
{'phase', 3.5, {default= {0, 0, 1, 0}, {0, 1, 2, 3}, {1, 2, 3, 4}, {2, 3, 4, 5}}}
```

### spline
Names: ```'spline'```

Constructs a spline from its operands and uses a t value to evaluate the
spline to calculate the result.

The first operand is the equation for the t value.  If the first operand is a
simple number, the result will never change.

All other operands are points on the spline.  The second operand is the spline value at t= 0, the third is the spline value at t= 1, and so forth.

This is a cubic spline, when t= 0, the result is the first point.  When t= 1,
the result is the second point, and thusly.

Because the spline points can be operands, they can also be equations or
sub expressions.  The system tracks whether an expression changes every
frame, every note, or never, and solves the spline when appropriate.

Adding ```loop= true``` will make the spline loop, which affects how the t
value is used, round splines wrap the t value to put in the range from zero
to the t value of the last point.

```polygonal= true``` changes the spline to use straight lines instead of
calculating curves.

```lua
-- Use the music beat as a t value.  When music beat is between 0 and 1,
-- follow a curve between zero and one.  When music beat is between 1 and 2,
-- curve between 1 and 4.  Music beat between 2 and 3 curves between 4 and 8.
-- Music beat greater than 4 follows a straight line.
-- The exact shape of the curve is affected by the spline values.
{'spline', 'music_beat', 0, 1, 4, 8, 16}
-- Straight changes instead of curves.
{'spline', polygonal= true, 'music_beat', 0, 1, 4, 8, 16}
-- Loop the spline, changing the curves's shape and shifting the t value to
-- the [0, 5) range.
{'spline', loop= true, 'music_beat', 0, 1, 4, 8, 16}
```

### lua
Names: No actual name, use a lua function as the first operand.

For when whatever you need cannot be expressed as an equation.  This will be
slightly slower than the equivalent equation, will not have built in spline
handling, and will not have the zero handling in divide and the rounding functions.

Operands are the things that need to be passed to the function.  Effectively,
this means the argumements to the function are stated twice, but the
alternative would be passing the lua function everything it could possibly
want, which would be unwieldy, and not allow passing the function the result
of some sub expression.

```lua
{function(beat, y_offset)
	return beat + y_offset
end,
'music_beat', 'y_offset'}
```


# Combining equations

Because each operand in an equation is a table, you can make a function that
creates the table for part of an equation and call that function in place of
an operand.

This is the code that implements the beat modifier.
```lua
equation= function(params, ops)
	-- triangle wave - n results in the section above zero having width 1-n
	-- so triangle - (1-w) will give us width w
	-- params.time is doubled and offset to put a peak on every beat.
	local triangle= {'triangle', {'+', .5, {'*', 2, params.time}}}
	local above_zero_is_width= {'-', triangle, {'-', 1, params.width}}
	-- height above zero needs to be 1, but is currently w.
	-- <result> / w will make the height 1
	local height_is_one= {'/', above_zero_is_width, params.width}
	-- then max is used to clip off the parts of the wave that are below
	-- zero, and we have a wave that is flat between peaks.
	local beat_wave= {'max', 0, height_is_one}
	local curved_wave= {'^', beat_wave, 2}
	-- inverter has peaks on even beats, and troughs on odd beats, to make
	-- the motion alternate directions.
	local inverter= {'square', {'+', .5, params.time}}
	local time_beat_factor= {'*', curved_wave, 20, inverter}
	local note_beat_factor= {
		ops.wave,
		{'+', params.wave_offset,
		 {'/',
			params.note_input,
			params.period,
		 },
		},
	}
	local time_and_beat= {ops.factor, time_beat_factor, note_beat_factor}
	return {ops.level, params.level, time_and_beat}
end

-- in the mods table.
	{column= 1, target= 'column_pos_y', start_beat= 8.25, length_beats= 2/16,
	 equation({level= 1, note_input= 'y_offset', wave_offset= .5, period= 6,
			time= 'music_beat', width= .5}, {level= '*', factor= '*', wave= 'sin'})
			},
```

Thus, once you have a concept or name for an action, you can put that in a
function that takes parameters for the changeable parts and reuse it.
