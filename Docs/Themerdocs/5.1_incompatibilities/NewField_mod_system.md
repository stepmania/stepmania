This document is about the system for controlling the appearance of notes in
the notefield.  Things that change how notes appear are also known as
modifiers.

## Overview
The mod system is a set of building blocks, each layer built on top of the
previous.  This guide will go from the bottom layer to the top.  
Mods are heavily based on abstraction, so prepare yourself.

Mods are designed to be "fire and forget".  You create a ```ModFunction``` by
calling ```ModifiableValue:add_mod()``` or a similar function, and then never
change it.  Functions for accessing and modifying ```ModFunction```s after
creation are provided, but should not be necessary.

There is a mod management system for handling mods that need to start or end
at a particular time.


#### Complexity
If some aspect is too complex or strange to understand, you can safely ignore
it and just use the parts you understand.  For example, if you have trouble
understanding the phases input shifter, you can ignore it and just make mods
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


# Classes
* ModInput
* ModFunction
* ModifiableValue
* ModifiableVector3
* ModifiableTransform
* NewFieldColumn
* NewField


## ModInput

### Explanation
```ModInput``` is a piece of input for a ```ModFunction```.  It is a type,
a scalar, an offset, and two ```input limiters```.  The type sets which
value from a note is used.  When a ModInput is evaluated, the value from the
note is put through the input limiters, then multiplied by the scalar, then
the offset is added.

### Input types
To render a note, the notefield takes a few basic properties from the note
and gives them to the modifiers as input.  Things that are not notes have the
current music beat and second for their EvalBeat and EvalSecond inputs, and 0
for their Dist inputs.  
* Scalar  
  The scalar value will be used, and not multiplied with a value from the
  note.
* EvalBeat  
  EvalBeat is the beat of the music that the note occurs on.
* EvalSecond  
  EvalSecond is the second of the music that the note occurs on.
* MusicBeat  
  MusicBeat is the current beat of the music.
* MusicSecond  
  MusicSecond is the current second of the music.
* DistBeat  
  DistBeat is EvalBeat - MusicBeat.
* DistSecond  
  DistSecond is EvalSecond - MusicSecond.
* YOffset  
  YOffset is the value calculated by the speed mod, before reverse effects
  are applied.  YOffset is zero for the speed mod, reverse mods, and receptor
  and explosion mods.
* StartDistBeat  
  The distance in beats from the start time of the ```ModFunction```.
* StartDistSecond  
  The distance in seconds from the start time of the ```ModFunction```.
* EndDistBeat  
  The distance in beats from the end time of the ```ModFunction```.
* EndDistSecond  
  The distance in seconds from the end time of the ```ModFunction```.

#### Why are there both Beat and Second inputs?
Because C mods exist.  And for other things that find it more convenient to
work with seconds instead of beats.

### Input shifters
To enable mods that need to shift the input around in special ways before
using it, input shifters are provided inside ModInput.  
To use an input shifter in a ModInput, put it inside the table when creating
the ModInput.

##### Input shifter order
To use input shifters, it helps to understand the steps they are applied in.
Each step has a piece of pseudocode to illustrate how the value changes in each step.
* 1. Initial input value is picked.  (Ex: ```y_offset= 5```)
* 2. Input value goes through rep input shifter.  (Ex: ```y_offset= rep(y_offset)```)
* 3. Input value goes through phase input shifter.  (Ex: ```y_offset= phase(y_offset)```)
* 4. Input value is multiplied by the scalar member of the ModInput.  (Ex: ```y_offset= y_offset * 1```)
* 5. Input value goes through the spline input shifter.  (Ex: ```y_offset= spline(y_offset)```)
* 6. The offset member of the ModInput is added in.  (Ex. ```y_offset= y_offset + 0```)
* 7. The final result is passed to the parent ModFunction.  (Ex: ```return y_offset```)

#### rep input shifter
The rep input shifter causes a specific sub-range of the input to repeat
instead of using the full range of the input.  It has only ```rep_begin```
and ```rep_end```.  The result of the rep input limiter will never equal ```rep_end```.
##### Example
```rep= {1, 2}```
rep_begin is 1.
rep_end is 2.
* input is 2, result is 1.
* input is .2, result is 1.2.
* input is -.2, result is 1.8.

#### phase input shifter
The phase input shifter applies a multiplier and an offset if the input is in
its range.  ```
Equation: ```result= ((input - phase_start) * multiplier) + offset```
The range includes the beginning, but not the end.  If the input is not in
the range of a phase, the default phase is used.  The default phase has a
start of 0, finish 0, mult 1, and offset 0 (so the input is unchanged).  
A ModInput can have multiple phases to simplify creating mods that need
multiple phases. (say, when beat ramps up the amplitude on a sine wave, then
ramps it down.  Between beats is one phase, ramp up is another, and ramp down
is a third.)  
The system relies on phases not overlapping and being sorted.  If the list of
phases is not sorted, or two phases overlap, the result is undefined.  
##### Example
```phases= {{0, 1, 2, -1}}```
phase_start is 0.  
phase_finish is 1.  
phase_mult is 2.  
phase_offset is -1.  
* input is -1, result is -1 (outside the phase).
* input is 0, result is -1 (0 - 0 is 0, 0 * 2 is 0, 0 + -1 is -1).
* input is .5, result is 0 (.5 - 0 is .5, .5 * 2 is 1, 1 + -1 is 0).
* input is 1, result is 1 (outside the phase).
```phases= {{0, 1, 2, -1}, {1, 2, 1, 0}}```
Two phases are in this example.  
* input is -1, result is -1 (outside any phase).
* input is 0, result is -1 (in phase one).
* input is .5, result is 0 (in phase one).
* input is 1, result is 0 (in phase 2).
* input is 1.5, result is .5 (in phase 2).
* input is 2, result is 2 (outside any phase).

#### Spline input shifter
If the input needs to follow a complex curve, a spline can be used.  
The optional fields ```loop``` and ```polygonal``` can be set to make the
spline looped or polygonal.  Each point on the spline must be a number.
##### Example
```{"ModInputType_MusicBeat", 1/4, spline= {loop= true, 0, 1.5, 2, 1.5}}```
This creates a looped spline with points at 0, 1.5, 2, and 1.5.  When the
```ModInput``` is evaluated, the music beat will be multiplied by 1/4, the
spline will evaluated at that point, and the result returned.  
So at music beat 0, the spline is evaluated at t= 0.  At music beat 4, the
spline is evaluated at t= 1.  Since the spline is looped, at music beat 16,
the spline is evaluated at t= 0, because t= 4 is equivalent to t=0 on a
looped spline.
```{"ModInputType_MusicBeat", 1/4, spline= {loop= true, polygonal= true, 0, 1.5, 2, 1.5}}```
This time the spline is polygonal instead of being curved.

### Functions
Function descriptions are omitted in cases where they would just restate what
the function name says.  
In the args for these functions ```phase``` means a table of four numbers:
```{start, finish, mult, offset}```.  
In general, the get and set functions for a thing return it exactly as it
appears when constructing a ModInput.  So if you create a ModInput with
```phases= {{0, 1, 2, -1}, {1, 2, 1, 0}, default= {-5, 5, -1, 1}}``` then
```get_all_phases()``` will return
```{{0, 1, 2, -1}, {1, 2, 1, 0}, default= {-5, 5, -1, 1}}``` and
```get_phase(1)``` will return ```{0, 1, 2, -1}```.
* get_type()
* set_type(ModInputType)
* get_scalar()
* set_scalar(float)
* get_offset()
* set_offset(float)
* get_rep()
  Returns ```{rep_begin, rep_end}```.
* set_rep({float, float})
  Pass nil to disable the rep input shifter.
* get_all_phases()
* set_all_phases(table)
* get_phase(int)
* set_phase(int, table)
* get_default_phase()
* set_default_phase(table)
* get_num_phases()
* remove_phase(int)
* clear_phases()
  Removes all phases and disables phase input shifter.
* get_enable_phases()
  Returns true if the phase input shifter is enabled.
* set_enable_phases(bool)
  Can be used to disable or enable the phase input shifter without changing
  any phases.
* get_spline_size()
* get_spline_point(int)
* set_spline_point(int, float)
* add_spline_point(float)
  Similar to set_spline_point, but adds to the end instead of setting a point
  that already exists.
* remove_spline_point(int)
* get_spline()
* set_spline(table)

### Construction Examples
These examples are meant as elements in a table of ```ModInput```s that are
being passed when creating a ```ModFunction```.  The first two examples are
equivalent, the bare number form is provided for convenience.  The first
element of the table must be the type, the second is the scalar, the third is
the offset.  If the scalar or offset is not provided, 0 will be used.  
The input limiters are optional and are set from named elements of the table.
``` 5 ```  
```{"ModInputType_Scalar", 5}```  
The second example is identical to the first, the simple number form is for
convenience.  
```{"ModInputType_EvalBeat", 1}```  
```{"ModInputType_EvalSecond", 2, 3}```
```{"ModInputType_DistBeat", 1, rep= {1, 2}}```
```{"ModInputType_DistBeat", 1, phases= {{0, 1, 2, 0}}}```
```{"ModInputType_DistBeat", 1, 2, rep= {1, 2}, phases= {{0, 1, 2, 0}, {1, 2, 4, 0}}}```


## ModFunction

### Explanation
A ```ModFunction``` takes some ```ModInput```s and returns a result.  The
number of ```ModInput```s and how they are used varies for each type of ```ModFunction```.  
A ```ModFunction``` can optionally have a name.  If a name is not provided,
one will be generated.  The name can be used to find the ```ModFunction```
in the ```ModifiableValue``` after creation.  

#### Managed Mods
If a ```ModFunction``` is added with ```add_managed_mod```, it should have a
start time and an end time.  The managing system will be discussed more in
the section for ```ModifiableValue```.  
The field for setting the start beat is ```start_beat``` and start second is
```start_second```.  End beat is ```end_beat``` and end second is ```end_second```.  
All the time fields are optional, but one for start and one for end should be
set.  If ```start_beat``` is not provided, it will be calculated from
```start_second```, and vice versa.  The same is true for the end time.

##### Start/End Dist Inputs
Managed mods can use the StartDistBeat, StartDistSecond, EndDistBeat, and
EndDistSecond inputs.  For unmanaged mods, those inputs will always be zero.

### Function types
* Constant  
  This takes a single input and returns it when evaluated.
* Product  
  This takes two inputs and multiplies them together.
* Power  
  This takes two inputs and returns ```base^exponent```.
* Log  
  This takes two inputs and returns ```log(value) / log(base)```, which is
  the log of ```value``` in base ```base```.
* Sine  
  This is a sine wave.  A sine wave has four inputs:  angle, phase,
  amplitude, and offset.  The equation looks like this:  
  ```return (sin(angle + phase) * amplitude) + offset```
* Tan  
  This is a tan wave.  You need a fresh tan before hitting the club.
* Square  
  This is a square wave.  The inputs are identical to a sine wave.
  For any angle and phase where ```sin(angle + phase)``` would be over zero,
  a square wave is 1.  Any angle and phase where ```sin(angle + phase)```
  would be under zero, a square wave is -1.  The amplitude and offset are
  applied in the same way as for a sine wave.
* Triangle  
  A triangle wave is like a sine wave, but with straight lines instead of
  curves.  The peaks and troughs of the triangle wave are exactly where they
  would be for a sine wave with the same inputs.
* Spline  
  ```ModFunctionSpline``` is the most complex.  It allows you to define a
  spline, then uses a ```ModInput``` to calculate a t value, the spline is
  evaluated at the t value, and the result is returned.  The spline is a one
  dimensional cubic spline.  Each point on the spline is a ```ModInput```.
  Using a input type that changes every frame like ```MusicSecond``` is
  practically free.  Using an input that is different for every note like
  ```EvalSecond``` is expensive, because the spline has to be solved again
  for every note that is rendered in the column.  Using per-note input for
  the t value is expected and incurs no performance penalty.  
  There are flags for creating the spline as a loop or polygonal. Lua.xml
  explains what makes a looped or polygonal spline special.  
  Changing the type of an input after creating the spline is guaranteed to
  make the spline do the wrong thing.  (on creation, the points are organized
  into different lists for scalar, per-frame, and per-note.  These lists are
  never updated)

### Functions
* get_inputs()  
  This returns the ```ModInputs``` in a table.  Because the different types
  have different inputs, here is a list of the table for each type.  The same
  input order is used when constructing a ```ModFunction``` for ```ModifiableValue:add_mod```.  
  * Constant: {value}
  * Product: {value, multiplier}
  * Power: {value, exponent}
  * Log: {value, base}
  * Sine: {angle, phase, amplitude, offset}
  * Square: Same as Sine.
  * Triangle: Same as Sine.
  * Spline: {t_value, point_1, point_2, ...}

### Construction Examples
These examples are meant as the argument to ```Modifiablevalue:add_mod```.  
A ```ModFunction``` can be given an optional name that can be used to fetch
it from the ```Modifiablevalue``` later.  If it is not given a name on
creation, a unique name will be generated.
```{"ModFunctionType_Constant", 2}```  
```{name= "fred", "ModFunctionType_Constant", 2}```  
```{"ModFunctionType_Constant", {"ModInputType_Scalar", 2}}```  
```{"ModFunctionType_Power", {"ModInputType_DistBeat", 1}, 2}```  
```{"ModFunctionType_Sine", {"ModInputType_DistBeat", 1}, 0, 2, 0}```  
```{"ModFunctionType_Constant", {"ModInputType_DistBeat", 2, rep= {0, .1}}}```  
```{"ModFunctionType_Constant", {"ModInputType_DistBeat", 2, phases= {default= {0, 0, 0, 1}, {.1, .9, 10, 0}}}}```
The most extreme example: https://youtu.be/b1jcIgLFlws
```
{name= "kloozorg", "ModFunctionType_Sine",
  {"ModInputType_EvalBeat", 1, .5, rep= {.2, .4}, phases= {
  {.2, .25, 0, 0}, {.25, .35, 4, 0}, {.35, .4, 0, 1}}},
  {"ModInputType_EvalSecond", 1, -.5, rep= {.4, .6}, phases= {
  {.4, .45, 0, 0}, {.45, .55, 4, 0}, {.55, .6, 0, 1}}},
  {"ModInputType_DistBeat", 1, .5, rep= {.2, .4}, phases= {
  {.2, .25, 0, 0}, {.25, .35, 4, 0}, {.35, .4, 0, 1}}},
  {"ModInputType_DistSecond", 1, -.5, rep= {.4, .6}, phases= {
  {.4, .45, 0, 0}, {.45, .55, 4, 0}, {.55, .6, 0, 1}}}
}
```
### Spline Construction Examples
This constructs a simple spline with 6 points, all of them scalars.  The
t value uses the y offset of the note as input, and multiplies it by 1/16,
so the note takes 96 pixels to traverse the whole spline.  If this were on
the x position of a note, it would be positioned normally at y offset 0, 4
pixels to the right at y offset 16, 16 pixels right at y offset 32, 64 pixels
right at y offset 48, 16 pixels right at y offset 64, 4 pixels right at y
offset 80, and back to 0 pixels right at y offset 96.
```
{"ModFunctionType_Spline", t= {"ModInputType_YOffset", 1/16}, loop= true,
  0, 4, 16, 64, 16, 4}
```
The second example is the same as the first, but it uses the current second
of the music as the input for the point in the middle of the spline.  This
makes that point move as the song progresses.
```
{"ModFunctionType_Spline", t= {"ModInputType_YOffset", 1/16}, loop= true,
0, 4, 16, {"ModInputType_MusicSecond", 2}, 16, 4}
```
The third example uses the beat the note is on as input for a point.  The
spline must be solved every time a note is rendered, which is expensive.
```
{"ModFunctionType_Spline", t= {"ModInputType_DistSecond", 1/16}, loop= true,
0, 16, {"ModInputType_YOffset", 1/16}, 16}
```
This example makes the spline polygonal just to demonstrate how to set that
flag.  A polygonal spline has straight lines instead of curved lines.
```
{"ModFunctionType_Spline", t= {"ModInputType_DistSecond", 1/16}, loop= true,
polygonal= true, 0, 16, {"ModInputType_YOffset", 1/16}, 16}
```

### Example
This example assumes that a ```ModFunction``` has been fetched from a
```ModifiableValue``` and stored in the variable ```mod```.
```
local inputs= mod:get_inputs()
for i, input in ipairs(inputs) do
  -- Call some ModInput function here.
  input:set_scalar(5)
end
```


## ModifiableValue

### Explanation
A ModifiableValue is a base value, and a list of ```ModFunction``` mods.
When evaluated, the base value and the mods are all summed together.

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
* add_get_mod(ModFunction)  
  This takes the same arg as ```add_mod``` and it returns the newly created
  ```ModFunction```.  ```add_get_mod``` is equivalent to using ```add_mod```
  and chaining it with ```get_mod```.
* get_mod(name)  
  This returns the ```ModFunction``` with the given name in the list.
* remove_mod(name)  
  Removes the ```ModFunction``` with the given name in the list.
* clear_mods()  
  Removes all ```ModFunction``` from the list.
* add_managed_mod(ModFunction)  
  Identical to add_mod, but for managed mods.
* add_managed_mod_set({ModFunction, ...})  
  Takes a table of ```ModFunction```s to add.
* add_get_managed_mod(ModFunction)  
  Identical to add_get_mod, but for managed mods.
* get_managed_mod(name)  
  Identical to get_mod, but for managed mods.
* remove_managed_mod(name)  
  Identical to remove_mod, but for managed mods.
* clear_managed_mods()  
  Identical to clear_mods, but for managed mods.
* get_value()  
  Returns the base value of the ```ModifiableValue```.
* set_value(float)  
  Sets the base value of the ```ModifiableValue```.

### Examples
This assumes that a ```ModifiableValue``` has been fetched from something and
stored in the variable ```mod_val```.
```
-- First mod:  1
-- Second mod:  The distance from the note to the receptor, in seconds.
-- Third mod:  The input angle for the wave is the beat * pi, and the
--   amplitude grows from 0 at the beginning to 1 two minutes in.  After 4
--   minutes, the amplitude would be 2.
-- Fourth mod:  Same as the third, but only uses the part of the sine wave
--   that is between pi*.25 and pi*.75.
mod_val:add_mod{"ModFunctionType_Constant", 1}
  :add_mod{"ModFunctionType_Constant", {"ModInputType_DistSecond", 1}}
  :add_mod{"ModFunctionType_Sine", {"ModInputType_DistBeat", pi}, 0,
    {"ModInputType_MusicSecond", 1/120}, 0}

local mod= mod_val:get_mod(3)
-- Imagine the example for ModFunction here.

mod_val:remove_mod(3)
Trace("There are " .. mod_val:num_mods() .. " mods in the list.")

mod_val:clear_mods()

mod_val:add_managed_mod_set{
  {start_beat= 18, end_beat= 34, "ModFunctionType_Constant", {"ModInputType_StartDistBeat", pi/2}},
 {start_beat= 38, end_beat= 54, "ModFunctionType_Constant", {"ModInputType_StartDistBeat", -pi/2}},
}
```


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
```ModifiableTransform``` for that.  If this is a serious problem, look up
the Actor function ```add_wrapper_state```, which puts an ActorFrame around
an actor that already exists.

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
  current time of the column.  The ```Eval*``` inputs for it are identical to
  the ```Music*``` inputs, and the ```Dist*``` inputs are 0.
* reverse_scale (ModifiableValue)  
  ```reverse_scale``` controls whether notes scroll up or down.  At 1, notes
	scroll up.  At -1, notes scroll down.
  ```reverse_scale``` is evaluated once per frame, with only the
  current time of the column.  The ```Eval*``` inputs for it are identical to
  the ```Music*``` inputs, and the ```Dist*``` inputs are 0.
* center_percent (ModifiableValue)  
  ```center_percent``` acts as an additional shift factor on the y offset
  that moves it towards the center of the column.  ```center_percent``` of 0
  leaves the y offset unchanged.  ```center_percent``` of 1 cancels out
  ```reverse_offset_pixels``` entirely, putting the receptor in the center of
  the column.  
  ```center_percent``` is evaluated once per frame, with only the
  current time of the column.  The ```Eval*``` inputs for it are identical to
  the ```Music*``` inputs, and the ```Dist*``` inputs are 0.
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
* note_alpha (ModifiableValue)  
* note_glow (ModifiableValue)  
* receptor_alpha (ModifiableValue)  
* receptor_glow (ModifiableValue)  
* explosion_alpha (ModifiableValue)  
* explosion_glow (ModifiableValue)  
  Notes, receptors, and explosions all have separate alpha and glow values.
  Alpha defaults to 1, glow defaults to 0.

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
  If set to false, the y offset will not be added to a note's position.  Use
  this when you are using get_note_trans_pos_y() mods to set the y position
  and the y offset is getting in your way.
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
* apply_note_mods_to_actor(actor)
  Sets the transform of the actor as if it were affected by the note mods.

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
column:get_speed_mod():add_mod{"ModFunctionType_Constant",
  {"ModInputType_DistSecond", play_bpm / 60}}

-- Set a speed mod that is equivalent to 2x.
column:get_speed_mod():add_mod{"ModFunctionType_Constant",
  {"ModInputType_DistBeat", play_x}}

-- Set a speed mod that is equivalent to m300.  Fetching the display bpm is
--   outside the scope of this document.
column:get_speed_mod():add_mod{"ModFunctionType_Constant",
  {"ModInputType_DistBeat", play_bpm / display_bpm}}

-- Note that if you actually do all three of these on a column, the speed mod
--   will be the sum of all three.  So for a chart with a display bpm of 150,
--   imagine a note that is 1 beat and 0.4 seconds away:
--     local cmod_result= 0.4 * 5  -- 5 is the result of 300 / 60
--     local xmod_result= 1 * 2  -- 2 is play_bpm
--     local mmod_result= 1 * 2
--     y_offset= (cmod_result + xmod_result + mmod_result) * 64
--   This puts the note 6 arrow heights away, 384 pixels.  With any one of
--   those speed mods, it would be only 4 arrow heights away, 128 pixels.


-- Put the notefield in reverse.
column:get_reverse_percent():set_value(1)

-- Make the notes follow a sine wave left and right as they go up.
column:get_note_pos_x():add_mod{"ModFunctionType_Sine",
  {"ModInputType_DistBeat", pi}, 0, 128, 0}

-- Make the column change size with the music, and have the size grow as the
--   song continues.  Calibrated to reach max amplitude at the end of the
--   song, regardless of length.  Fetching the length of the song is outside
--   the scope of this document.
--   1/130 is used so the x zoom doesn't quite hit zero at the end of the
--   song.
local seconds_factor= 120 / song_length
column:get_column_zoom_x():add_mod{"ModFunctionType_Triangle",
  {"ModInputType_MusicBeat", pi/2}, 0,
  {"ModInputType_MusicSecond", 1/130 * seconds_factor}, 0}
```


## NewField

### Tweening
Attempting to use the normal Actor functions for setting the position,
rotation, or zoom of a ```NewField``` will have no effect.  There is a
```ModifiableTransform``` for that.  If this is a serious problem, look up
the Actor function ```add_wrapper_state```, which puts an ActorFrame around
an actor that already exists.

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
  receptor_alpha and receptor_glow affect layers with even draw order.
  explosion_alpha and explosion_glow affect layers with odd draw order.

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

-- Tilt the field, similar to what distant does, but do it on a SawTriangle
--   wave, so it starts with no tilt, gradually tilts, then snaps back.
field:get_trans_rot_x():add_mod{"ModFunctionType_SawTriangle",
  {"ModInputType_MusicBeat", pi/16}, 0, -pi/4, 0, 0, pi/2}

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
```{"ModFunctionType_Constant", {"ModInputType_DistBeat", 2}}```
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
Remove the long prefixes from the strings used for types.  This is a problem
with Stepmania's string enum system in general, it requires the name of the
enum as a prefix on the strings passed in from Lua.

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
