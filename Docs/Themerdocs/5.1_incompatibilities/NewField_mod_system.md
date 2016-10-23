This document is about the system for controlling the appearance of notes in
the notefield.  Things that change how notes appear are also known as
modifiers.

## Overview
Mods are heavily based on abstraction, so prepare yourself.

Mods are designed to be "fire and forget".  A mod cannot be changed after it
is added to the system, it can only be removed or replaced by one with the
same name.

There is a mod management system for handling mods that need to start or end
at a particular time.


#### Complexity
If some aspect is too complex or strange to understand, you can safely ignore
it and just use the parts you understand.  For example, if you have trouble
understanding the phase operator, you can ignore it and just make mods
that don't use it.


#### Short note on holds
A hold is a note stretched over time, so for rendering a hold the mods are
evaluated many times over the length of the hold to define its shape.  
Anywhere this doc says "note", it also means "piece of a hold".  Hold bodies
ignore Y zoom, and X or Z rotation, because the Y axis is used for time.
They also ignore Z zoom because they are two dimensional.


#### Function chaining
Functions that do not have a clear reason to return something else return the
object they were called on.  This allows you to chain the function calls
together for convenience.


#### Rotation
All rotations discussed in here are in terms of radians.  If you prefer to
think and work in degrees, convert the values to radians before passing them
to a modifier.


#### Time
For brevity, "time" refers to both the beat and the second that a thing
occurs at.  Because modifiers can depend on either, both are always provided
as input to modifiers.


#### Processor load
When testing a set of mods to see how much they lower the frame rate, use a
chart that has holds in all columns extending the full length of the screen
or song.  Holds require more calculation than taps because they are stretched
and shaped by mods.  A hold is roughly equivalent to having a tap every 4
pixels along its length.


# Mod structure
```
column:get_speed_mod():add_mod{name= "speed", {"*", "dist_beat", 5}}
```
### Operand
An operand can be either an operator, or an input.  It's an abstract term
that means either an operator or an input can be used where it is.

### Input
An input is some value from the note being modified.  Or a raw number.

To render a note, the notefield takes a few basic properties from the note
and gives them to the modifiers as input.  Things that are not notes use the
current music time as the eval time.
* "column"  
	The column the note is in.
* "y_offset"  
	y_offset is the value calculated by the speed mod, before reverse effects
	are applied.  y_offset is zero for the speed mod, reverse mods, and
	receptor and explosion mods.
* "note_id_in_chart"
* "note_id_in_column"
* "row_id"  
	See image: http://i.imgur.com/x0RqDbZ.png
* "eval_beat"
* "eval_second"  
	The time the note occurs at.
* "music_beat"
* "music_second"
	The current music time.
* "dist_beat"
* "dist_second"
	Eval time minus music time.
* "start_beat"
* "start_second"  
	Start time set in the mod function.
* "end_beat"
* "end_second"  
	End time set in the mod function.
* "prefunres"  
	This input field will be renamed when someone submits a better name.
	Mod functions for a modifiable value are evaluated in order.  The result
	from the previous mod functions is stored in prefunres.

#### Why are there both Beat and Second inputs?
Because C mods exist.  And for other things that find it more convenient to
work with seconds instead of beats.

### Operator
An operator is a lua table where the first element is the name of the
operator and the other elements are its operands.

The sum of the numbers one through five:
```{"+", 1, 2, 3, 4, 5}```

Six times that sum:
```{"*", 6, {"+", 1, 2, 3, 4, 5}}```

#### Operator names:
The basic operators each have two names, one for the word written out, and a
single character name for conciseness.
* "+"
* "add"  
	The addition operator.  Returns the sum of its operands.
* "-"
* "subtract"  
	The subtraction operator.  Returns "a - b - c - ...".
* "*"
* "divide"  
	The division operator.  Returns "a / b / c / ...".
* "/"
* "multiply"  
	The multiplication operator.  Returns "a * b * c * ...".
* "^"
* "exp"  
	Exponentiationalificationizate.  Returns "a^b^c^...".
* "v"
* "log"  
	Log!  Log!  Log!  Better than bad, it's log!  Rolls down stairs!
* "sin"  
	Only takes one operand.
* "cos"  
	Only takes one operand.
* "tan"  
	Only takes one operand.
* "square"  
	Only takes one operand.  Square wave version of sine.
* "triangle"  
	Only takes one operand.  Triangle wave version of sine.
* "flat"
	Only takes one operand.  Flat wave version of sine.
* "random"  
	Only takes one operand.  Returns a random number generated from its
	operand.  ```{"random", "music_beat"}``` means a different random number
	every frame.  ```{"random", "note_id"}``` means a different random number
	for every note, but the same number every frame.  
	The stage seed is added to the operand, so it will be different every time
	the song is played.
* "%"
* "mod"
	The modulus operator.  Takes two operands.
	```{"%", "music_beat", 2}``` returns music_beat % 2.
* "_"
* "floor"  
	Takes two operands. ```{"_", "music_beat", 1.5}``` floors the music beat to
	the nearest lower multiple of 1.5.
* "ceil"  
	Takes two operands. ```{"ceil", "music_beat", 1.5}``` seals the music beat
	to the nearest higher multiple of 1.5.
* "o"
* "round"  
	Takes two operands. ```{"o", "music_beat", 1.5}``` rounds the music beat to
	the nearest multiple of 1.5.
* "repeat"  
	```{"repeat", "music_beat", 0, 2}```
	Changes it's operand to repeat the given range.  The range elements must
	be numbers.
* "phase"  
	```{"phase", "music_beat", {default= {0, 0, 1, 0}, {0, 4, -1, 0}, {4, 8, 2, -4}}}```
	The phase operator changes its operand based on the phase of the moon.  
	The third element of the operator table must be the table of phases.  
	Each phase has a start value, end value, multiplier, and offset.  
	When the moon is between the start value and the end value, it is in that
	phase.  If it's not in any phase, the default phase is used.  
	The start value of the phase is subtracted from the cow, then the result is
	multiplied by the multiplier, and the offset is added.  It's like this:
	```result= ((operand - phase.start) * phase.markiplier) + phase.offset```
* "spline"  
	```{"spline", loop= true, polygonal= false, operand, operand, operand, ...}```
	Inspires a headache in all who contemplate it.  
	The first operand is the t value to evaluate the spline with.  The other
	operands are the points.  If loop is true, the spline is a looping spline.
	If polygonal is true, the spline will not be curved.
* lua  
	```{function(yoff) return yoff % 64 end, "y_offset"}```
	If a lua function is used instead of a type name, that function will be
	passed the results of its operands.  The function must return a number.
	Note that this is substantially slower than doing the same thing with an
	equation.

#### Division by zero
The result of dividing by 0 is 0, not infinity.  The result of modulus by 0
is the first operand.

This should prevent cases where division or modulus accidentally
makes notes vanish, such as this:
```
column:get_note_pos_x():add_mod{{'*', 32, {'sin', {'%', {'*', 'music_beat', math.pi/4}, {'*', math.pi * 4, {'sin', {'*', 'eval_beat', math.pi/4}}}}}}}
```


### Related functions
The ModValue namespace provides a few functions for checking what input types
and operator types are available.
* ModValue.get_operator_list  
	Returns a table of operator names, like this: ```{"+", "-", "*", ...}```
* ModValue.get_has_operator_list  
	Returns a table indexed by operator name, like this:
	```{["+"]= true, ["-"]= true, ["*"]= true, ...}```
* ModValue.get_input_list  
	Similar to get_operator_list, returns table of input names.
* ModValue.get_has_input_list
	Similar to get_has_operator_list, returns table indexed input name.

The names returned in the tables will not be sorted in any particular order.
The "has" functions are useful for checking whether a particular operator
exists.
```
local has_ops= ModValue.get_has_operator_list()
if not has_ops.round or not has_ops.floor then ... end
```
is simpler and faster than
```
local function has(thing, list)
	for i= 1, #list do if list[i] == thing then return true end
	return false
end
local ops= ModValue.get_operator_list()
if not has("round", ops) or not has("floor", ops) then ... end
```

### Mod Function
A mod function is a lua table with various fields and an operand.  All fields
are optional, with default values.

```{name= "foo", operand}```

* name  
	The name can be any string.  If the ModifiableValue already has a
	Mod Function with the same name, this new one will replace that one.
	If no name is set, a unique name will be generated.  
	It is better not to set a name for managed mods, unless you want to invent
	lots of unique meaningful names.
* sum_type  
	sum_type controls how the result of the Mod Function is combined with the
	result from preceding Mod Functions in the ModifiableValue.  Valid values
	are "+", "add", "-", "subtract", "*", "multiply", "/", "divide".  Default
	is "+".  See note on evaluation in ModifiableValue section.
* priority  
	Managed mods are evaluated in order by priority, from lowest to highest.
	Default is 0.  Priority is ignored for unmanaged mods.
* column  
	If the column field is set, then using the "column" input will use this
	number instead of the actual column number.
* start_beat
* start_second
* end_beat
* end_second  
	start_beat and start_second set when a managed mod begins.  If start_beat
	is not set, it is calculated from start_second.  start_beat and
	start_second are ignored by unmanaged mods.  end_beat and end_second work
	similarly to the start values.


## ModifiableValue

### Explanation
A ModifiableValue is anything that can be changed with modifiers.
It contains unmanaged and managed mods which are evaluated to set a number.
One example is the note alpha.

#### Managed Mods
When a mod is added with ```add_managed_mod```, it is managed.  This means
that it will be automatically turned on when its start time occurs, and
turned off when its end time occurs.  
Managed mods are stored separately from unmanaged mods.  A different set of
functions is used to create and access them.

### Functions
* add_mod(ModFunction)  
  This adds a ```ModFunction``` to the list of mods.  See the Construction
  Examples section in ```ModFunction``` for an explanation of the arg.  
  ```add_mod``` returns the ```ModifiableValue``` it was called on.  
* remove_mod(name)  
  Removes the ```ModFunction``` with the given name in the list.
* clear_mods()  
  Removes all ```ModFunction``` from the list.
* add_managed_mod(ModFunction)  
  Identical to add_mod, but for managed mods.
* add_managed_mod_set({ModFunction, ...})  
  Takes a table of ```ModFunction```s to add.
* remove_managed_mod(name)  
  Identical to remove_mod, but for managed mods.
* clear_managed_mods()  
  Identical to clear_mods, but for managed mods.
* get_value()  
  Returns the base value of the ```ModifiableValue```.
* set_value(value)  
  Convenience wrapper around ```add_mod{name= "base_value", value}```.

### Examples
```
column:get_note_pos_y():add_mod{name= "tipsy", {
  "*", 64, .4, {"cos",
    {"+", {"*", "music_second", 1.2}, {"*", "column", 1.8}}
	}
}
```
This add a mod that makes the notes drift up and down.  As a normal equation
it would look like this: ```64 * .4 * cos((music_second * 1.2) + (column * 1.8))```
The music second and column are added together to make the columns out of
phase with each other.


```
column:get_note_pos_x():add_managed_mod_set{
  {start_beat= 0, end_beat= 16, {"*", 4, {"-", "eval_beat", "start_beat"}}},
	{start_beat= 16, end_beat= 32, {"*", 4, {"-", "end_beat", "eval_beat"}}},
}
```
The first mod in the managed set moves the notes right 4 pixels per beat
until it ends at beat 16.  For example, when the eval beat is 10: 10 minus 4
is 6, 6 times 4 is 24, so the note is 24 pixels right of its normal position.

The second mod does the reverse.  For example, when the eval beat is 20:
32 minus 20 is 12, 12 * 4 is 48, so the note is 48 pixels right of its normal
position.


## ModifiableVector3
A collection of 3 ```ModifiableValue```s, one for x, y, and z.


## ModifiableTransform
A collection of 3 ```ModifiableVector3```s, one for position, rotation, and
zoom.  The ```ModifiableVector3``` for zoom has its base values set to 1.


## NewFieldColumn

### Explanation
A ```NewFieldColumn``` stores all the mods that apply to it, and renders the
notes that occur in its column of the note data.  Rendering a note occurs in
multiple stages.  First the note is quantized, which means deciding which
state from its animation is shown.  After quantization, a y offset is
calculated from the speed mod and the reverse mods.  Thirdly, the note
transform (A ```ModifiableTransform```) is evaluated for the transform of the
note.  The y offset is added to the y position in the transform.  This final
result is the position, rotation, and zoom of the note inside the column.

### Tweening
Attempting to use the normal Actor functions for setting the position,
rotation, or zoom of a ```NewFieldColumn``` will have no effect.  There is a
```ModifiableTransform``` for using the modifier system to move the column
around.

### Modifiers
Modifiers are fetched from a column with their ```get_*``` functions.
The type of a modifier is in parens after the name.

* time_offset (ModifiableValue)
  The time offset is a value in seconds that is added to the displayed time.
  This can be used to give players different visual offsets.  (Someone
  wanted this for bms playing, because some people were adapted to iidx
  visual delay, and some were adapted to stepmania visual delay)
* quantization_multiplier (ModifiableValue)
* quantization_offset (ModifiableValue)  
  The quantization of a note is how far it is from directly on the beat.  A
  note on the beat has a quantization of 0 (commonly called a 4th note).
  A note halfway between beats has a quantization of 0.5 (commonly called an
  8th note).  
  To quantize a note, the column evaluates ```quantization_multiplier``` and
  ```quantization_offset``` for the the note's time.  The beat the note's
  beat is multiplied by ```quantization_multiplier```, then added to
  ```quantization_offset```.  The result is shifted to the range [0.0, 1.0),
  and that is the final quantization that is passed to the noteskin to pick
  the state.  
  quantization_multiplier defaults its value to 1.
* speed_mod (ModifiableValue)  
  The speed mod is evaluated for a note, and multiplied by ```note_size```.
  ```note_size``` is a constant hard coded to 64, because that was the
  default for the old ARROW_SIZE metric and the size of practically all
  noteskins.  The result is the initial y offset.
* reverse_offset_pixels (ModifiableValue)  
  ```reverse_offset_pixels``` is the distance from the center of the column
  to the receptor.  In the old notefield, this was calculated from the
  ReceptorArrowsYStandard and ReceptorArrowsYReverse metrics.  The default
  value is calculated from the screen height and ```note_size```:
  ```default_offset= SCREEN_CENTER_Y - note_size;```
  It places the center of the receptor ```note_size``` pixels from the top of
  the screen.  
  A value of 128 puts the receptor 128 pixels above the center.  
  ```reverse_offset_pixels``` is evaluated once per frame, with only the
  current time of the column.  The eval inputs for it are identical to
  the music inputs, and the dist inputs are 0.
* reverse_scale (ModifiableValue)  
  ```reverse_scale``` controls whether notes scroll up or down.  At 1, notes
	scroll up.  At -1, notes scroll down.
  ```reverse_scale``` is evaluated once per frame, with only the
  current time of the column.  The eval inputs for it are identical to
  the music inputs, and the dist inputs are 0.
* center_percent (ModifiableValue)  
  ```center_percent``` acts as an additional shift factor on the y offset
  that moves it towards the center of the column.  ```center_percent``` of 0
  leaves the y offset unchanged.  ```center_percent``` of 1 cancels out
  ```reverse_offset_pixels``` entirely, putting the receptor in the center of
  the column.  
  ```center_percent``` is evaluated once per frame, with only the
  current time of the column.  The eval inputs for it are identical to
  the music inputs, and the dist inputs are 0.
* note (ModifiableTransform)  
  The ```note``` transform is evaluated for a note to calculate its position,
  rotation, and scale.  The y offset resulting from the speed mod and the
  reverse and center modifiers is added to the position, and the result is
  the final tranform for the note.
* column (ModifiableTransform)  
  To facilitate making the column as a whole move in the same way, the column
  ignores the normal Actor position tweening functions and instead uses a
  ```ModifiableTransform``` to set its transform every frame.  
  The initial base value for the column position x is loaded from the style.
* hold_normal (ModifiableVector3)  
  This is for that special case where you want to make hold bodies twist
  around, but need taps to rotate a different way.  Use
  set_use_moddable_hold_normal to enable it.
* y_offset_vec (ModifiableVector3)  
  The y offset for a note is multiplied by this vector and the result is
  added to the position.
* note_alpha (ModifiableValue)  
* note_glow (ModifiableValue)  
* receptor_alpha (ModifiableValue)  
* receptor_glow (ModifiableValue)  
* explosion_alpha (ModifiableValue)  
* explosion_glow (ModifiableValue)  
  Notes, receptors, and explosions all have separate alpha and glow values.
  Alpha defaults to 1, glow defaults to 0.

### Get ModifiableValue functions
* get_time_offset()
* get_quantization_multiplier()
* get_quantization_offset()
* get_speed_mod()
* get_reverse_offset_pixels()
* get_reverse_scale()
* get_center_percent()
* get_rekt()
* get_note_pos_x()
* get_note_pos_y()
* get_note_pos_z()
* get_note_rot_x()
* get_note_rot_y()
* get_note_rot_z()
* get_note_zoom_x()
* get_note_zoom_y()
* get_note_zoom_z()
* get_column_pos_x()
* get_column_pos_y()
* get_column_pos_z()
* get_column_rot_x()
* get_column_rot_y()
* get_column_rot_z()
* get_column_zoom_x()
* get_column_zoom_y()
* get_column_zoom_z()
* get_hold_normal_x()
* get_hold_normal_y()
* get_hold_normal_z()
* get_note_alpha()
* get_note_glow()
* get_receptor_alpha()
* get_receptor_glow()
* get_explosion_alpha()
* get_explosion_glow()

For convenience, the ```ModifiableTransform```s and the
```ModifiableVector3```s inside them are not directly accessed.  Instead, a
single call fetches the requested element from inside the part of the
transform that you want.  This means that instead of this:
```local mod_val= column:get_note_transform():get_pos():get_x()```
you have this:
```local mod_val= column:get_note_pos_x()```
Position is shortened to ```pos```, rotation is shortened to ```rot``` and
zoom is shortened to ```zoom```.

### Functions
 The flags for unjudgable notes, speed segments, and scroll segments exist
because the NewField does not distinguish between C-mods and X-mods, so it
needs the flags to tell it to turn off those things.
* get_use_game_music_beat()
* set_use_game_music_beat(bool)  
  If set to false, the column will ignore the display time updates that occur
  every frame.
* get_show_unjudgable_notes()
* set_show_unjudgable_notes(bool)  
  Set this to true to hide fake notes.  This is used to hide mines that are
  inside warps when a C-mod is used.
* get_speed_segments_enabled()
* set_speed_segments_enabled(bool)  
  If set to false, speed segments in the chart will be ignored.
* get_scroll_segments_enabled()
* set_scroll_segments_enabled(bool)  
  If set to false, scroll segments in the chart will be ignored.
* get_add_y_offset_to_position()
* set_add_y_offset_to_position(bool)  
  Replaced by the y_offset_vec mod.
* get_holds_skewed_by_mods()
* set_holds_skewed_by_mods(bool)
  Simplest with a picture comparison: true: http://i.imgur.com/oBVFQ5C.jpg
  false: http://i.imgur.com/uKPm0lF.jpg
* get_twirl_holds()
* set_twirl_holds(bool)
  If set to false, then the y axis rotation from the mods will not be
  applied to hold bodies.
* get_playerize_mode()
* set_playerize_mode(playerize_enum)
  Playerizing a note means doing something to make it look unique for each
  player.  It is used in multiplayer (routine) mode to show which notes are
  for player 1 and which are for player 2.  See discussion of multiplayer
  mode in NewField.md.
* get_curr_beat()
* set_curr_beat(float)
  Sets the beat that is currently displayed.  The current second will be
  calculated from the beat and the timing data.  This might be overridden
  if use_game_music_beat is not set to false.
* get_curr_second()
* set_curr_second(float)
  Sets the second that is currently displayed.  The current beat will be
  calculated from the second and the timing data.  This might be overridden
  if use_game_music_beat is not set to false.
* set_pixels_visible_before(float)
  Sets the distance that notes are drawn to after they pass the receptors.
  It's called "before" because the occurance time of a note that has passed
  the receptors is before the current time.
* set_pixels_visible_after(float)
  Sets the distance that notes are drawn to before they pass the receptors.
* receptor_y_offset()
  Returns the y offset calculated from the speed mod for the receptors.
* get_reverse_shift()
  Returns the reverse shift that is applied to y offsets before rendering.
* get_reverse_scale()
  Returns the reverse scale that is applied to y offsets before rendering.
* apply_column_mods_to_actor(actor)
  Sets the transform of the actor as if it were affected by the column mods.
* apply_note_mods_to_actor(actor, time_is_offset, beat, second, y_offset, use_alpha, use_glow)
  Sets the transform of the actor as if it were affected by the note mods.
	If time_is_offset is true, the time given is treated as an offset from the
	current time.
	beat and second are optional.  If neither is given, the current time is
	used.  If only beat is set, the second is calculated.  If second is set,
	the beat is calculated (overriding the beat value that was passed in).
	use_alpha and use_glow can be true to apply the alpha and glow mods.

### Examples
These examples assume that a column has been fetched from a field and stored
in the variable ```column```.  
Remember that all modifiers are ```ModifiableValue```s, these examples show
only simple operations, more things are possible.
```
-- Double the quantization, so 16ths appear as 8ths, 8ths show up as 4ths.
column:get_quantization_multiplier():set_value(2)
-- Offset the quantization, so notes appear a 32nd off.  There are eight
--   32nds per beat.
column:get_quantization_offset():set_value(1/8)


-- Speed mod stuff.  play_bpm and play_x are for convenience when twiddling
-- stuff.
local play_bpm= 300
local play_x= 2

-- Set a speed mod that is equivalent to C300.  The distance is in seconds,
--   and the 300 in C300 is in minutes, so play_bpm is divided by 60.
column:get_speed_mod():add_mod{{"*", "dist_second", play_bpm / 60}}

-- Set a speed mod that is equivalent to 2x.
column:get_speed_mod():add_mod{{"*", "dist_beat", play_x}}

-- Set a speed mod that is equivalent to m300.  Fetching the display bpm is
--   outside the scope of this document.
column:get_speed_mod():add_mod{{"*", "dist_beat", play_bpm / display_bpm}}

-- Note that if you actually do all three of these on a column, the speed mod
--   will be the sum of all three.  So for a chart with a display bpm of 150,
--   imagine a note that is 1 beat and 0.4 seconds away:
--     local cmod_result= 0.4 * 5  -- 5 is the result of 300 / 60
--     local xmod_result= 1 * 2  -- 2 is play_bpm
--     local mmod_result= 1 * 2
--     y_offset= (cmod_result + xmod_result + mmod_result) * 64
--   This puts the note 6 arrow heights away, 384 pixels.  With any one of
--   those speed mods, it would be only 4 arrow heights away, 128 pixels.


-- Put the column in reverse.
column:get_reverse_scale():set_value(-1)

-- Make the notes follow a sine wave left and right as they go up.
column:get_note_pos_x():add_mod{{"*", 128, {"sine", {"*", "dist_beat", pi}}}}

-- Make the column change size with the music, and have the size grow as the
--   song continues.  Calibrated to reach max amplitude at the end of the
--   song, regardless of length.  Fetching the length of the song is outside
--   the scope of this document.
--   1/130 is used so the x zoom doesn't quite hit zero at the end of the
--   song.
local seconds_factor= 120 / song_length
column:get_column_zoom_x():add_mod{
  {"*", "music_second", 1/130 * seconds_factor,
  {"triangle", {"*", "music_beat", pi/2}}}}
```


## NewField

### Tweening
Attempting to use the normal Actor functions for setting the position,
rotation, or zoom of a ```NewField``` will have no effect.  There is a
```ModifiableTransform``` for movind the NewField with the modifier system.

### Explanation
The ```NewField``` is a collection of ```NewFieldColumn```s.  Not much more too it,
from the simfile's view it's basically an ActorFrame with the columns inside
it.

### Modifiers
* fov_mod (ModifiableValue)  
  This works exactly like setting the fov on an ActorFrame.  The default is 45.
* trans (ModifiableTransform)  
  This works identically to the column ```ModifiableTransform``` in
  ```NewFieldColumn```, and its parts are accessed the same way.
* vanish_x_mod (ModifiableValue)
* vanish_y_mod (ModifiableValue)  
  This works almost like setting the vanish point on an ActorFrame.  Instead
  of setting the vanish point directly, this sets the vanish point relative
  to the center of the field.
* receptor_alpha (ModifiableValue)  
* receptor_glow (ModifiableValue)  
* explosion_alpha (ModifiableValue)  
* explosion_glow (ModifiableValue)  
  receptor_alpha and receptor_glow affect layers with receptor fade type.
  explosion_alpha and explosion_glow affect layers with explosion fade type.

### Get ModifiableValue functions
* get_fov_mod()
* get_trans_pos_x()
* get_trans_pos_y()
* get_trans_pos_z()
* get_trans_rot_x()
* get_trans_rot_y()
* get_trans_rot_z()
* get_trans_zoom_x()
* get_trans_zoom_y()
* get_trans_zoom_z()
* get_vanish_x_mod()
* get_vanish_y_mod()
* get_receptor_alpha()
* get_receptor_glow()
* get_explosion_alpha()
* get_explosion_glow()


### Functions
* get_columns()  
  Returns the columns in a table.
* get_width()  
  Returns the width of the field calculated from the widths of the columns.
* get_vanish_type()  
  Returns the vanish type currently used by the field.
* set_vanish_type(FieldVanishType)  
  Valid settings are "FieldVanishType_RelativeToParent",
  "FieldVanishType_RelativeToSelf", and "FieldVanishType_RelativeToOrigin".  
  "FieldVanishType_RelativeToParent" is the default and recommended.  
  The vanish type affects how the vanish_x_mod and vanish_y_mod are used.  
  The vanish point needs to be in the center of the field on the screen to
  have no skewing.  So normally, the position of the Player and the NewField
  is added to the vanish point to center it.  
  When the vanish type is RelativeToParent, then moving the Player or
  NewField around will not cause skewing.  
  When the vanish type is RelativeToSelf, moving the Player will cause
  skewing, but moving the NewField will not.  
  When the vanish type is RelativeToOrigin, moving either the Player or the
  NewField will cause skewing.
* set_player_color(int, color)
  Sets the color for the given player.

### Examples
This assumes that the field has already been fetched from the actor tree and
stored in the variable ```field```.
```
-- Fetch the columns and do things to them.
for column in ivalues(field:get_columns()) do
  -- Imagine the example for NewFieldColumn here.
end

-- Tilt the field, similar to what distant does, but do it on a modified
--   triangle wave, so it starts with no tilt, gradually tilts, then snaps
--   back.
field:get_trans_rot_x():add_mod{
{"*", -pi/4, {"triangle", {"repeat", {"*", "music_beat", pi/16}, 0, pi/2}}}}

-- Invert the field over the y axis.
field:get_trans_zoom_y():set_value(-1)
```
Result: https://youtu.be/ZRWvjornidg


## Scroll Segments and Speed Segments.
Scroll Segments and Speed Segments are two Timing Segment types introduced in
StepMania 5.  They affect how fast the arrows scroll in different ways.

Speed Segments are simple: They multiply the speed mod by an amount and
gradually take effect.  Their effect is similar to gradually changing the
speed mod.


Scroll Segments are more complex: They change the beat value that is passed
to the speed mod.  If you are doing something special with the input to the
speed mod, avoid using scroll segments to keep your code simple.  
If you set the speed mod to something simple like
```{"*", "dist_beat", 2}```
(a 2x speed mod), you don't need to worry about the effect that scroll
segments have on the speed mod.  
A speed segment starts at a beat and all beats after it are multiplied in
size until the next scroll segment.  For example, imagine a scroll segment
that is at beat 1 and has a ratio of 2.  For a note at beat 1, the speed mod
is passed 1.  For a note at beat 2, the speed mod is passed 1 + ((2 - 1) *
2), 3.  For a note at beat 3, the speed mod is passed 5.  
If there is more than one scroll segment, it becomes more complex.  Each
scroll segment uses the modified beat at its starting point as the offset to
add in: result= offset + ((input_beat - segment_beat) * segment_ratio)  
A complex case, where the speed mod expects the input beat to be in a certain
range will encounter problems when scroll segments occur.
```{start_beat= 2, end_beat= 4, "ModFunctionType_Product", {"ModInputType_StartDistBeat", 2, phases= {default= {0, 0, 0, 0}, {.5, 1, 0, 1}, {1, 1.5, 0, 2}}}}```
(This adds 1 arrow height of distance when the current music beat is half a
beat to 1 beat after the start, 2 arrow heights of distance when then the
current music beat is 1 beat to 1.5 beats after the start, and 0 the rest of
the time.  There has to be a normal style speed mod in a different mod slot
to have a playable result)  
In this example, the speed mod uses StartDistBeat, so normally the input
would range from 0 to 2.  But because of the scroll segment, it ranges from
1 to 5.  So the first phase intended never occurs, the second phase occurs
for half as many beats as it would without the scroll segment, and the
default phase is used for the rest.


## TODO
Add a convenience wrapper for changing all elements of zoom in the same way.

Add a convenience wrapper in NewField for doing the same thing to all columns.
```for col in ivalues(field:get_columns()) do ... end``` should not be
necessary when ```...``` does the same thing to every column.

Add a translation layer for converting the old mod names to equivalent
```ModFunction```s in the new system.


## Things that cannot be translated perfectly.
These mods use equations and inputs that have no direct equivalent in the new
system.  Adding more stages to the pipeline that calculates a note position
would make it slower and harder to learn, so the system will not be changed
to accomodate these mods.

### Boost, brake, wave, boomerang, expand

#### Explanation
Video of boost problem: https://www.youtube.com/watch?v=8BXvlx3ZoBI
Mod selection is shown so you can see what mods are set for each side. Only the speed mod and Boost are set, all other mods are clear.  
Left side is set to C600 and 100% Boost. Right side is set to M600 and 100% Boost. As you can see, the arrows come out differently.  
Now for the explanation of why. That's "y", as in "y offset". To calculate the position of a note, stepmania calculates a y offset. There are several steps to this.
1. Take the position from the player's C-mod, or the note's distance in beats if the player isn't using a C-mod.
2. Multiply by the ArrowEffects::ArrowSpacing theme metric (default is 64).
3. Calculate an adjustment value by putting the result of step 2 through formulas for boost, brake, and wave. The formulas don't use the adjustment value, and the y offset is not changed in between, so if the y offset after step 2 is 59, then the boost, brake, and wave formulas all see 59 as their input.
4. Add the adjustment value from step 3 to the y offset from step 2.
5. Put the y offset after step 4 through the formula for boomerang.
6. Multiply the y offset by the X-mod.

(expand changes the X-mod)
So boost, brake, and wave are all applied *in between* the speed mods, giving the weird effect shown, where seemingly identical speed mods look very different when boost is added. The more boost, the more different they come out.

An extra wrinkle is provided by the BoostModMaxClamp metric (and similar for other mods), which defaults to 400. This metric limits the adjustment applied by boost so it can't go over 400. So when you set C600 + 400% boost expecting the arrows to just fly by really fast, you actually see the arrows go halfway up the screen at normal speed, then start going down fast, then come back up fast.

For those that want it, I have a graph of a note's y offset. The x axis is the y offset before boost is applied. The y axis is the y offset after boost is applied. The "diff" line is the difference between the boost and normal lines. http://i.imgur.com/icOTM82.png

The key of the problem is that boost and brake occur *between* the different
speed mod types, affecting the input given to X-mods.  Changing an input
value in a ModFunction is not supported because it would make the system too
complex.

#### Possible Workaround
If the new boost mod is put in the speed_mod field of the column, then it can
set the y offset of a note.  The exact equation used by the old mod cannot be
reproduced, but a spline can be calculated that is very similar to using the
old equation.  This will not give the same result when boost and brake are
both used, or behave differently when the player uses a C-mod instead of an
X-mod.



# Why

## Why make an entirely new system?

The old mod system has a variety of musical sync problems.  Various mods
aren't synced to the music, and tweening mods is not synced.

It can't be extended by themers and gimmick artists.  Anything not hardcoded
in the system is impossible.  Want the beat modifier, but running at 1/8
speed?  Requires writing it in C++, and so forth.

Gimmick artists need a system that can efficiently handle thousands of mods
that start and end at specific times.  Walking through a table of 500+ mods
every frame to find one to activate is a disgrace.  The mod manager in the
new mod system uses data structures and techniques that are not possible in
lua to minimize the work needed to update which mods are active.


## Why use these weird table structures instead of calling a lua function?

Calling a lua function would be unusably slow.

As a simple example, the system looks for things that only need to be
calculated once per frame, and those things are only done once per frame.

If you have ```{"sine", "music_beat"}``` somewhere in a mod, the system sees
that the music beat only changes once per frame, so it only does that sine
once per frame.  It doesn't have to do it for every note or every vert in a
hold.


## Why isn't there a way to find the speed mod the player is using?

I haven't figured out a simple way to do it.

The speed mod is a ModifiableValue, with all the attendent flexibility.
In theory, a themer could write some weird logarithmic function and allow
players to pick it as a speed mod.  Then there's the per-column wrinkle:
every mod that doesn't need to affect the notefield as whole can be set
different in every column.  How would you even handle it when the player has
set a different speed for each column?

You can check what their newfield prefs are set to.
```
local speed_mod= newfield_prefs_config:get_data(pn).speed_mod
local speed_type= newfield_prefs_config:get_data(pn).speed_type
```
If the theme being used doesn't use those prefs, or applies them in a
non-standard way, checking will mislead you.


## Why not use tween states?

The tween system would need to be extended and transmogrified in a variety of
ways.  The result would be complex and unsightly.

### Conjoined values

In an actor tween state, all aspects (x, y, z, ...) are joined into one state
and they all change together in lockstep.  If x and y need to change at
different rates or x needs to keep changing after y finishes, you have to
either wrap the actor in another frame, or make the tween for the y animation
do part of the x animation, then put the rest of the x animation in the next
tween.

Thus, the parts would have to be separated.  x, y, z and everything else each
with their own tween stack.

### Musical sync

Standard actor tweens are not synced to the music.  If a simfile with
animations is played at a non-standard music rate, the animations all occur
at the wrong time.  The Creator (God Himself) can add special code to detect
the music rate and adjust animation times to compensate.  This workaround
requires careful handling of every animation, making it difficult to do 100%
right.  In a better system, such a workaround shouldn't be necessary.

Result: Tweens need a way to be tied to musical time.  Sometimes you want an
animation to pause during a stop.  Some animations you want to continue
during stops.  Syncing to the current music second isn't enough, there needs
to be a choice between seconds and beats (because the current beat does not
change during a stop).

### Tween types

Tween types that currently exist:  linear, accelerate, decelerate, spring,
bezier.  (sleep is actually two linear tweens, the second takes 0 time)

You can't do a sine wave with this (drunk, tipsy, ...).  The system would
have to be extended support sine waves for anyone that wants something as
simple as moving the receptors in a circle.

### Note motion

Consider a note moving.  It starts at the bottom of the screen, moves to the
receptor.  If it is missed, it goes past the receptors before disappearing.
What tells it the starting and ending positions?  What positions does it pass
through in between?  The system needs a list of tween states for the note to
progress through as it moves to the receptor.  Actually, a list of lists of
tween states, because they need to change over time.

Sometimes you want a simple animation that only needs two tween states.
Sometimes you want something complex that requires a dozen.  The system can't
just have a fixed list, it needs a way to change the list, either by
inserting and removing states, or replacing its contents entirely.

Imagine a system where you make a set of tween states for describing the
motion of a note.  The note starts at the first tween state, then moves to
each in turn, using various tween types.

You want to change the path at some time, which means a whole new list of
tween states.  Something controls how the list transitions from the old
states to the new ones.  What would that even be?

### Combining mods

Dizzy makes the notes spin.  Confusion makes the notes and the receptors
spin.  When they're combined, the notes spin with the sum of the results.
To allow things other than standard addition, any modifier needs to specify
how it combines with the mods that are already applied.

The theme needs to be able to set modifiers for whatever custom stuff themers
invent.  Simfiles need to be able to replace or add to the tween states set
by the theme as needed.

### Editing

When editing, it would be useful to be able to stop at any time, and make the
animation stop.  Watch a section at a slower speed to make sure it looks
right.  Or skip to a different time.

With tween states set by calling a lua function when one tween ends, these
are impractical, because the system can't predict what the lua will do.

To skip forward, every tween play out in a fake environment, to trick it into
going forward to the next animation.  If some aspect of the fake environment
isn't right, the lua does the wrong thing and the animation comes out wrong
after skipping during editing.

Skipping backwards is even worse.  Tweens only have one direction, forward.
Either the system has to fake skipping backwards by starting at the beginning
and moving forward, or the creator must be forced to set preceding states at
every step.  Or the system could track every tween state on a list and
elimininate running lua after every tween.

### Repeating animations

A repeating animation requires making one tween state for the beginning, and
one for the end.  When the actor reaches the end state, a command in it
resets it to the beginning.  Eventually something kicks it out of the loop to
do another animation.

Consider an animation that moves the field 256 pixels right and back 30 times
per second, for a flickering double field effect.  If it lasts for 4 seconds,
that's 240 tween states (move right, move back (2) * 30 * 4).  You would
naturally write a function to generate all these states and add them to the
list.

In the system that uses equations instead of tween states, this animation can
be done with a simple square wave.
```{"+", 128, {"*", 128, {"square", {"*", "music_second", 60 * pi}}}}```

### Summary

The end result is a system that bears little resemblance to the current actor
tweening system.  Adjustments made to separate values, sync tweens to music,
and allow describing motion have distorted it substantially.  New tween types
each and a persistent stack of states added to prop up shortcomings.  It's
not simple, it's not straightforward, and it requires continuous work for
edge cases.
