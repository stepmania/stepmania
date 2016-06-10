# What is NewField?
NewField is a complete replacement for the NoteField, NoteSkin, and modifier
systems, written from scratch.  The new systems are designed to be more
flexible and predictable and documented on the user side, and easier to
maintain on the developer side.


# Compatibility
Noteskins made for the old system must be converted by a human to be used in
the NewField.  Typically this involves remaking the hold graphics and writing
some new lua code to replace the metrics.

Themes made for the old system should work without changes.  Old themes will
not allow setting advanced NewField options such as noteskin parameters until
updated by their authors.

Simfiles made to set modifiers in the old mod system will work to varying
degrees.  They should be tested by their authors to clean up rough edges.
In particular ApplyGameCommand has been removed because it is obsolete and
doesn't do the right thing for gimmicks anyway.

## FastNoteRendering
The FastNoteRendering preference is obsolete.  The NewField uses a different
method for rendering arrows that prevents the intersection problem that
occurred with 3D notes and FastNoteRendering, and doesn't slow the game down
the way turning off FastNoteRendering did in the old NoteField.

## Under the Hood
Compatibility is provided through a complete rewrite of ArrowEffects, called
ArrowDefects.  When something touches the PlayerOptions structure for a
player, the field for that player is put into "defective mode".  When the
field is in that mode, it ignores all the new modifier fields and instead
only uses the stuff in PlayerOptions.  This should make old mods behave the
same in the NewField as in the old NoteField.  This includes unwanted things
such as theme metrics for receptor position and screen height making some
mods appear different in different themes.

### Musical Sync
Because the NewField is based around having mods synced to music, mods that
did not have musical sync in the old system are synced in ArrowDefects.
This means that when these mods are used in conjunction with a music rate
mod, the mod effect will happen at a different rate.  It should make them
behave more reliably as well.
* Expand used a global variable to accumulate time that was not reset between
  songs.  Playing a song after waiting a different amount of time would make
  the notes expand and contract at slightly different times.  In
  ArrowDefects, Expand uses the current music second, minus the total length
  of any stops.
* Blink, Drunk, and Tipsy used the time since stepmania started up, and did
  not stop when the game was paused.  In ArrowDefects, these mods use the
  current music second.


# Making a theme use NewField

## NoteField actor removed
The old NoteField has been removed from the Player actor on ScreenGameplay.
The Player:set_newfield_preferred function no longer exists because it no
longer has a purpose.

## NewField preferences
The _fallback theme provides a structure for preferences for the NewField.
These preferences cover normal things like speed and hidden, to more exotic
choices such as y offset (distance from field center to receptors) and
whether notes flash white before disappearing when hidden is on.

### Enabling NewField preferences

#### Loading and Saving
The preferences should be automatically loaded and saved when profiles are
loaded and saved.  If your theme has custom profile loading and saving
functions, read Docs/Themerdocs/lua_config_system.md and look for the section
"Loading and Saving".  That will tell you to write different functions and
use add_profile_load_callback and add_profile_save_callback instead of having
LoadProfileCustom and SaveProfileCustom functions in the theme.

#### Options Screen
NewField preferences cannot be set through the old metrics style commands
that set the old player options.  Instead, they must be set through lua
option rows or a custom lua menu system.  The _fallback theme provides a
custom lua menu system, and parts for building menus to set the preferences.
The default theme has an (uncommented) example options screen that uses the
custom lua menu system named "ScreenNestyPlayerOptions".

Using old metrics style commands on a normal options screen to set modifiers
will trigger the backwards compatibility auto detection and disable the
newfield prefs.

#### Gameplay
Add an actor to a layer on ScreenGameplay like this:
```lua
t[#t+1]= use_newfield_actor()
```
This will create an actor that will apply the newfield preferences for both
players.  The newfield preferences will be discussed in their own
documentation file.

#### Music Select
Call ```reset_needs_defective_field_for_all_players()``` on ScreenSelectMusic
to reset the defective mode flag for the players.  If the flag is not reset,
then when someone plays a simfile that triggers the defective mode detection,
they'll be stuck in that mode until they start a new credit.


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
There is a table named newfield_draw_order with entries for receptors and
explosions so newskins can have a conventional draw order for each.

Anything that needs to be at a fixed position in the field should be in a
layer file, either in NoteField layers, or NoteColumn layers.  This means
screen filters, column flashes, lane graphics, anything  that is in the
field.

That way, the thing is guaranteed to be in the correct position for the layer
and column that it is in.  If mods move the field or the column around, the
layers move too.

In general, when picking the draw order for something, put it halfway between
the things it should be between.  That leaves the most space for other things
to be added in between it.  This is why the engine elements are in units of
100, and named variables are provided at the mid points.

To put the judgment underneath all the notes, but not part of the board, put
it at 50.  Putting the judgment at 350 would put it above all the notes.  250
would put it above hold bodies, but underneath taps.

#### ActorProxy
Drawing the Player or the NewField or a NewFieldColumn with an ActorProxy
will bypass some of the draw order logic.  Things with a draw order less than
0 will not be under other theme elements when an ActorProxy is used.

### Judgment/Combo

The judgment and combo actors will not use the JudgmentUnderField and
ComboUnderField metrics when the NewField is in use.

Instead, the judgment and combo actors should have a draw order set, as if
they were layers in the field.  Consider this example:  The combo has a draw
order of 50, and the judgment has a draw order of 450.  All of the field
layers with a draw order less than 50 are rendered, then the combo, then
everything between 50 and 450, then the judgment.  This puts the combo above
the screen filter, underneath the notes, and the judgment above the notes.

Using the draw order instead of the metrics allows the theme to set a
different draw order for each player.  One player can have their judgment
above the notes while the other has their judgment under the notes.

The draw order of the judgment and combo actors is checked every frame, so if
it changes during gameplay, the change takes effect immediately.


### Alpha/glow and transform mods

Each layer has a fade type to control which alpha and glow mods affect it.
If the fade type is set to FieldLayerFadeType_Receptor, then the layer is
affected by the receptor alpha/glow mods.  FieldLayerFadeType_Explosion sets
it to use the explosion alpha/glow mods.  FieldLayerFadeType_Note will make
it use the note alpha/glow mods (with only the current music beat and second
for inputs).  FieldLayerFadeType_None is for layers that should not be
affected by any mod.  The default is FieldLayerFadeType_None.  For field
layers, FieldLayerFadeType_Note is treated as FieldLayerFadeType_None.

The layer fade type is set with the set_layer_fade_type function, in the
column or field.  If the layer is in a column, call set_layer_fade_type on
the column.  If the layer is in the field, call set_layer_fade_type on the
field.  The simplest place to call it from is the WidthSetCommand, because
that command is run immediately after the layer is loaded, and has the field
or column in the param table.

Column layers also have a transform type, to control whether the layer is
place at the center of the column, or at the receptor position.  Field layers
do not have a transform type because the base field doesn't have a receptor
position.  FieldLayerTransformType_Full uses the full receptor transform,
position, scale, and rotation.  FieldLayerTransformType_PosOnly only uses
the position, which can be useful for a column flash effect that extends from
the receptor towards the center.  FieldLayerTransformType_None does not use
the head transform at all.  FieldLayerTransformType_None is the default.

The layer transform type is set by set_layer_transform_type.  WidthSetCommand
is also the best place to call it.

#### Example WidthSetCommand
For a column layer:
```lua
WidthSetCommand= function(self, param)
  param.column:set_layer_fade_type(self, "FieldLayerFadeType_Explosion")
    :set_layer_transform_type(self, "FieldLayerTransformType_HeadPosOnly")
end
```
For a field layer:
```lua
WidthSetCommand= function(self, param)
  param.field:set_layer_fade_type(self, "FieldLayerFadeType_Explosion")
end
```


#### Draw Order Variable List

02 NewField.lua defines these variables to make life convenient for themers.
```lua
newfield_draw_order= {
	layer_spacing= 100,
	mid_layer_spacing= 50,
	board= -100,
	mid_board= -50,
	non_board= 0,
	under_field= 50,
	receptor= 100,
	over_receptors= 150,
	hold= 200,
	between_taps_and_holds= 250,
	tap= 300,
	under_explosions= 350,
	explosion= 400,
	over_field= 450,
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
table contains the width, padding, and x position of a column.
* field:  The NewField the board is attached to.  If you need it for some
reason, this is how to get it.

#### Example
Imagine that the engine does this to create the param for WidthSetCommand:
```lua
local width_info= {
  field= self,
  width= 256,
	columns= {
	  {width= 64, padding= 0, x= -96},
	  {width= 64, padding= 0, x= -32},
	  {width= 64, padding= 0, x= 32},
	  {width= 64, padding= 0, x= 96},
	},
}
```
So to size a quad to go behind the notefield, you just use param.width as the
width of the quad, and pick a height.  Note that if the noteskin sets custom
positions for the columns and doesn't center them, a centered quad won't be
behind the columns.  Complain to the noteskin author.  
To size a set of quads, each one in a different column, walk through the
columns table.

## NewFieldColumn layer messages

The various layers in the column, from both the noteskin and the theme, are
sent various messages to keep their state updated.

### PlayerStateSetCommand
The param table has one element, param.PlayerNumber.  This is the number of
the player the field is for.

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
* ```sign``` -1 if the column is in reverse mode, 1 if it's not.

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


# Non-modifier Functions

## NewFieldColumn Functions

* NewFieldColumn:get_layer_fade_type(layer)  
Returns the FieldLayerFadeType for the layer.

* NewFieldColumn:set_layer_fade_type(layer, type)  
layer must be the actor the type is being applied to.  type is a
FieldLayerFadeType enum value.

* NewFieldColumn:get_layer_transform_type(layer)  
Returns the FieldLayerTransformType for the layer.

* NewFieldColumn:set_layer_transform_type(layer, type)  
layer must be the actor the type is being applied to.  type is a
FieldLayerTransformType enum value.

## NewField Functions

* NewField:get_layer_fade_type(layer)  
Identical to NewFieldColumn:get_layer_fade_type.

* NewField:set_layer_fade_type(layer, type)  
Identical to NewFieldColumn:set_layer_fade_type.

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

## Other functions

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
