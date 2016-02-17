# What is NewField?
NewField is a complete replacement for the NoteField, NoteSkin, and modifier
systems, written from scratch.  The new systems are designed to be more
flexible and predictable and documented on the user side, and easier to
maintain on the developer side.

Due to the completely new design, nothing is compatible.


# Making a theme use NewField
For transition period builds, both the old NoteField and the NewField exist
on ScreenGameplay.  The NewField is disabled unless the theme calls special
functions to enable it, and a NewSkin cannot be chosen without the right
option row being added to the metrics.

## Enabling

Add an actor to a layer on ScreenGameplay like this:
```
t[#t+1]= use_newfield_actor()
```
This will create an actor that will enable the newfield for both players and
apply their newfield preferences.  The newfield preferences will be discussed
in their own documentation file.

### Converting Speed, Distant, and Mini
The convert_oldfield_mods function is provided to convert the speed, distant,
mini, and scroll mods that are set in PlayerOptions to mods on the newfield.
Call it in an OnCommand and pass it the screen, the player number, and the
y offset like this:
```
for i, pn in ipairs(GAMESTATE:GetEnabledPlayers()) do
  convert(oldfield_mods(SCREENMAN:GetTopScreen(), pn, 100)
end
```
That function will take the Tilt, Mini, Alternate, Cross, Reverse, Split,
and speed mods from the PlayerOptions for the player and convert them to the
new system.

### Newskin Option Row
For themes that still use a metrics based options screen, a lua option row
for setting the newskin is provied.
```
LineNewSkin="lua,newskin_option_row()"
```
The default theme no longer uses a metrics based player options screen.


## NewField Layers
The "Graphics/NoteField board" file is not loaded by the NewField.  Instead,
NewField has a layers system.

The NewField loads "Graphics/NoteField layers", which returns a table of
actors (not an ActorFrame of actors).  These actors become children of the
field, and the field tracks their draw order.  Each column loads layers from
the noteskin, and from "Graphics/NoteColumn layers".  The field also tracks
the draw order of the layers in the columns.  During rendering, the field
renders everything by draw order, in all columns.  Everything at draw order 0
in all columns is drawn before anything at draw order 1 in any column.  This
allows more generalized control over exactly where things are drawn.


### Draw Order

Anything with a draw order less than 0 is considered part of the "board", and
is drawn before everything except the song background on ScreenGameplay.
This means they will be under things that the notes go over.

Hold bodies have draw order 200.  Taps have draw order 300.
NewSkin.receptor_draw_order is 100, and NewSkin.explosion_draw_order is 399,
so newskins can have a conventional draw order for each.

The judgment and combo and anything else that needs to be at a fixed position
in the field should be in a layer file, either in NoteField layers, or
NoteColumn layers.  That way, the thing is guaranteed to be in the correct
position for the layer and column that it is in, regardless of any mods that
may move the field or the column around, with no further effort.

To put the judgment underneath all the notes, but not part of the board, put
it at 51.  Putting the judgment at 351 would put it above all the notes.  251
would put it above hold bodies, but underneath taps.


#### Draw Order and alpha/glow mods

NewFieldColumn has mods for controlling the alpha and glow of receptors and
explosions.  Any layer in a column with an even draw order is treated as a
receptor.  Any layer in a column with an odd draw order is treated as an
explosion.  If the draw order integer is neither even nor odd, it will not
use either pair of mods.

This allows a judgment hiding mod to also hide tap explosions that reveal the
judgment, if the noteskin author gives everything that shows judgment an odd
draw order.

NewField also has receptor and explosion alpha and glow mods.  The same even
and odd draw order rules also apply to layers in the field.  Additionally,
any layer with a draw order of -100 or less cannot have its alpha or glow
changed by mods.

The extra rule allows board elements that are affected by mods, and board
elements that are not.

#### Draw Order Variable List

02 NewField.lua defines these variables to make life convenient for themers.
```
newfield_draw_order= {
	non_alphable_layer= -100,
	non_board= 0,
	receptor= 100,
	hold= 200,
	tap= 300,
	explosion= 399, -- Odd to make it use explosion alpha and glow mods.
}
```

## NewField layer messages

The NewField sends various info to the layers to make sizing simpler.
The info is sent through commands that are executed on the layers.  Note that
these commands are executed before the screen is finished loading.  You
cannot use SCREENMAN:GetTopScreen() during them.

### PlayerStateSetCommand
The param table has one element, param.PlayerNumber.  This is the number of
the player the field is for.

### WidthSetCommand
The param table has three entries:
* width:  The width in pixels from the left edge of the leftmost column to
the right edge of the rightmost column.
* columns:  A set of tables, one for each column, from left to right.  Each
table contains the width and padding of a column.
* newfield:  The NewField the board is attached to.  If you need it for some
reason, this is how to get it.

#### Example
Imagine that the engine does this to create the param for WidthSetCommand:
```
local width_info= {
  newfield= self,
  width= 256,
	columns= {
	  {width= 64, padding= 0},
	  {width= 64, padding= 0},
	  {width= 64, padding= 0},
	  {width= 64, padding= 0},
	},
}
```
So to size a quad to go behind the notefield, you just use param.width as the
width of the quad, and pick a height.  
To size a set of quads, each one in a different column, walk through the
columns table.

## NewFieldColumn layer messages

The various layers in the column, from both the noteskin and the theme, are
sent various messages to keep their state updated.

### WidthSetCommand
WidthSet is sent immediately after the OnCommand is executed.  The elements
in the param table are:
* ```column``` The NewFieldColumn the layer is in.
* ```column_id``` The id of the column.
* ```width``` The width of the column set by the noteskin.
* ```padding``` The padding set by the noteskin.

### ReverseChangedCommand
ReverseChanged is sent when the reverse scale goes from negative to positive,
or from positive to negative.  It is not sent every frame.
Param table elements:
* ```sign``` -1 if reverse_scale is less than 0, 1 otherwise.

### BeatUpdateCommand
BeatUpdate occurs every frame.  Param table elements:
* ```beat``` The current beat.
* ```beat_distance``` Distance in beats to the next note in this column.
* ```second_distance``` Distance in seconds to the next note in this column.
* ```pressed``` True when the column goes from not pressed to pressed.
* ```lifted``` True when the column goes from pressed to not pressed.

### ColumnJudgmentCommand
ColumnJudgment occurs when something is judged in the column.
Param table elements:
* ```bright``` True if the player's combo is greater than the
BrightGhostComboThreshold metric.
* ```tap_note_score``` If the judgment is for a tap note, this is the
judgment.  This is nil for holds.
* ```hold_note_score``` If the judgment is for a hold note, this is the
judgment.  This is nil for taps.

### HoldCommand
Hold occurs every frame when there is an active hold passing over the
receptors.  Param table elements:
* ```type``` The TapNoteSubType of the hold.
* ```life``` The current life value for the hold.  0 is a dropped hold, 1 is
a hold that is currently being held, in between is a hold that is going from
held to dropped.
* ```start``` True if this is the first frame the hold is active.
* ```finished``` True if this is the frame after the hold ended.


# Making a noteskin for the NewField
Read the comments in NewSkins/default.
_fallback/Scripts/02 NewSkin.lua has a couple minor functions for generating
state maps.

## 3D noteskins

3D noteskins are not supported on the NewField in this release.

# Newskin Parameters

Newskin parameters is a system for allowing a newskin to be customized by the
player.  The parameters are defined by the newskin and can do whatever the
skin author wants.  The player's choices are saved in their profile by skin
and stepstype (so the same skin can be set different ways for different
stepstypes).  The theme uses information provided by the newskin to give the
player a menu for setting the parameters.

The system is explained in more detail in comments in the default noteskin,
with examples used to control explosion particles and the amount of warning
time given by the receptors.

Because the newskin parameter table can go to any depth and contain anything,
and different players can choose different skins, creating a menu for it in
the confined OptionRow system would probably be very difficult.  OptionRow
menus for setting newskin parameters are not provided.  The nested option
menus system does provide menus for setting newskin parameters.


# Multiplayer mode
(aka: routine mode)
Playerizing a note means doing something to make it look unique for each
player.  It is used in multiplayer (routine) mode to show which notes are
for player 1 and which are for player 2.

Playerizing is on hold until someone has ideas and feedback.
Ideal playerizing should make it possible for notes to be playerized
and quantized and animated without forcing the noteskin author to multiply
the number of quantizations by the number of players by the frames of
animation.

There are currently two methods for playerizing notes.  The playerizing
method is set by the NewFieldColumn.

```NotePlayerizeMode_Off``` turns off playerizing of notes.  
```NotePlayerizeMode_Quanta``` turns of quantizing notes and instead uses
a different quanta for each player.  
```NotePlayerizeMode_Mask``` turns on a colored mask over the notes.  The
mask is a different color for each player.

If the theme attempts to set NotePlayerizeMode_Mask and the noteskin does not
support it, NotePlayerizeMode_Quanta will be used instead.

NewSkins/routine/notes.lua is an example of noteskin that supports
multiplayer mode.  It has example masks and colors and explanation comments.


# Functions in 02 NewField.lua

## NewField functions

* NewField:set_speed_mod(constant, speed, read_bpm)  
set_speed_mod creates a simple speed mod from the parameters and applies it
to all columns.  If ```constant``` is true, bpm changes, stops, speed
segments, and similar stuff will be disabled, as in an old-style CMod, and
```speed``` is the number of arrow heights a note should move per minute.
If ```constant``` is false and ```read_bpm``` is a number, ```read_bpm```
will be used to calculate a speed mod that makes notes at that bpm move at
```speed``` arrow heights per minute (this is the same as an old-style MMod).
If ```read_bpm``` is nil, it will be treated as 1, which makes it the same as
an old-style XMod.  If ```read_bpm``` is not nil and not a number, a theme
error will occur.

* NewField:set_rev_offset_base(revoff)  
Sets the base value for reverse_offset_pixels for all columns to revoff.

* NewField:set_reverse_base(rev)  
Sets the base value for reverse_scale for all columns to rev.

* NewField:clear_column_mod(mod_field_name, mod_name)  
Clears the mod with mod_name in the field mod_field_name in all columns.

* NewField:set_hidden_mod(line, dist, add_glow)  
Adds a hidden mod, which makes notes disappear as they cross the line.  line
is the y offset the mod effect is occurs at.  dist is how many pixels notes
take to go from fully visible to fully invisible.  If add_glow is true, then
the glow on notes will ramp up before they fade out.  If add_glow is false,
then the hidden mod will only make the notes transparent to fade them out.

* NewField:set_sudden_mod(line, dist, add_glow)  
Similar to set_hidden_mod, this adds a sudden mod, which makes notes after
line invisible, and notes above it visible.

* NewField:clear_hidden_mod()  
Clears the hidden mod from all columns.

* NewField:clear_sudden_mod()  
Clears the sudden mod from all columns.

* find_pactor_in_gameplay(screen_gameplay, pn)  
Returns the player actor or nil.

* find_newfield_in_gameplay(screen_gameplay, pn)  
Returns the newfield actor or nil.

* use_newfield_on_gameplay()  
Finds the player actors for all enabled players and sets them to render the
newfield instead of the oldfield.  Also applies the newfield prefs to their
newfields.

* use_newfield_actor()  
Returns an actor that calls use_newfield_on_gameplay in its OnCommand.

* newskin_option_row()  
Returns a lua option row for picking a newskin on a metrics based options
screen.
