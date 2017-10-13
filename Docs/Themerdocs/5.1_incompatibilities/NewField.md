# What is NewField?
NewField is a complete replacement for the NoteField, NoteSkin, and modifier
systems, written from scratch.  The new systems are designed to be more
flexible and predictable and documented on the user side, and easier to
maintain on the developer side.


# Compatibility
Noteskins made for the old system must be converted by a human to be used in
the NoteField.  Typically this involves remaking the hold graphics and writing
some new lua code to replace the metrics.

Themes made for the old system will have a variety of problems.
* No setting notefield prefs.
* No setting noteskin parameters.
* No setting noteskin.

Simfiles made to set modifiers in the old mod system will work to varying
degrees.  They should be tested by their authors to clean up rough edges.
In particular ApplyGameCommand has been removed because it is obsolete and
doesn't do the right thing for gimmicks anyway.

## FastNoteRendering
The FastNoteRendering preference is obsolete.  The NoteField uses a different
method for rendering arrows that prevents the intersection problem that
occurred with 3D notes and FastNoteRendering, and doesn't slow the game down
the way turning off FastNoteRendering did in the old NoteField.

## Under the Hood
Compatibility is provided through a complete rewrite of ArrowEffects, called
ArrowDefects.  When something touches the PlayerOptions structure for a
player, the field for that player is put into "defective mode".  When the
field is in that mode, it ignores all the new modifier fields and instead
only uses the stuff in PlayerOptions.  This should make old mods behave the
same in the NoteField as in the old NoteField.  This includes unwanted things
such as theme metrics for screen height making some mods appear different in
different themes.

Receptor offset in defective mode is hardcoded to the ITG values.

### Musical Sync
Because the NoteField is based around having mods synced to music, mods that
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
* PlayerOptions now uses the current music rate when tweening mods.  If haste
  is on, the rate is adjusted for haste.


# Adapting a theme to the new NoteField

## NoteField preferences
The _fallback theme provides a structure for preferences for the NoteField.
These preferences cover normal things like speed and hidden, to more exotic
choices such as y offset (distance from field center to receptors) and
whether notes flash white before disappearing when hidden is on.

### Enabling NoteField preferences

#### Loading and Saving
The preferences should be automatically loaded and saved when profiles are
loaded and saved.  If your theme has custom profile loading and saving
functions, read Docs/Themerdocs/lua_config_system.md and look for the section
"Loading and Saving".  That will tell you to write different functions and
use add_profile_load_callback and add_profile_save_callback instead of having
LoadProfileCustom and SaveProfileCustom functions in the theme.

#### Options Screen
NoteField preferences cannot be set through the old metrics style commands
that set the old player options.  Instead, they must be set through lua
option rows or a custom lua menu system.

The _fallback theme provides a custom lua menu system, and parts for building
menus to set the preferences.

The default theme has an (uncommented) example options screen that uses the
custom lua menu system named "ScreenNestyPlayerOptions".

Using old metrics style commands on a normal options screen to set modifiers
will trigger the backwards compatibility auto detection and disable the
notefield prefs.

#### Gameplay
Add an actor to a layer on ScreenGameplay like this:
```lua
t[#t+1]= notefield_prefs_actor()
```
This will create an actor that will apply the notefield preferences for both
players.  The notefield preferences will be discussed in their own
documentation file.

#### Music Select
Call ```reset_needs_defective_field_for_all_players()``` on ScreenSelectMusic
to reset the defective mode flag for the players.  If the flag is not reset,
then when someone plays a simfile that triggers the defective mode detection,
they'll be stuck in that mode until they start a new credit.


### Noteskin Option Row
For themes that still use a metrics based options screen, a lua option row
for setting the noteskin is provied.
```
LineNoteskin="lua,noteskin_option_row()"
```
The default theme no longer uses a metrics based player options screen.


## NoteField Layers
The "Graphics/NoteField board" file is not loaded anymore.  Instead,
NoteField has a layers system.

The NoteField loads "Graphics/NoteField layers", which returns a table of
actors (not an ActorFrame of actors).  These actors become children of the
field, and the field tracks their draw order.  Each column loads layers from
the noteskin, and from "Graphics/NoteColumn layers".

The field also tracks the draw order of the layers in the columns.  During
rendering, the field renders everything by draw order, in all columns.
Everything at draw order 0 in all columns is drawn before anything at draw
order 1 in any column.  This allows more generalized control over exactly
where things are drawn.

### Draw Order

Anything with a draw order less than 0 is considered part of the "board", and
is drawn before everything except the song background on ScreenGameplay.
This means they will be under things that the notes go over.

Hold bodies have draw order 200.  Taps have draw order 300.
There is a table named notefield_draw_order with entries for receptors and
explosions so noteskins can have a conventional draw order for each.

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
Drawing the Player or the NoteField or a NoteFieldColumn with an ActorProxy
will bypass some of the draw order logic.  Things with a draw order less than
0 will not be under other theme elements when an ActorProxy is used.

### Judgment/Combo

The judgment and combo actors will not use the JudgmentUnderField and
ComboUnderField metrics anymore.

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

### Hold Judgments

Hold judgments are judgments meant to appear below the receptors when a hold
finishes.  They should be put in the NoteColumn layers file.

FieldLayerTransformType_PosOnly can be used to position an ActorFrame on the
receptor, then the judgment sprite can be inside that frame and set a
position relative to the receptor to appear below.

ReverseChangedCommand can be used to reposition the actor for reverse mode.

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

02 NoteField.lua defines these variables to make life convenient for themers.
```lua
notefield_draw_order= {
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

## NoteField layer messages

The NoteField sends various info to the layers to make sizing simpler.
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
* field:  The NoteField the board is attached to.  If you need it for some
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

## NoteFieldColumn layer messages

The various layers in the column, from both the noteskin and the theme, are
sent various messages to keep their state updated.

### PlayerStateSetCommand
The param table has one element, param.PlayerNumber.  This is the number of
the player the field is for.

### WidthSetCommand
WidthSet is sent immediately after the OnCommand is executed.  The elements
in the param table are:
* ```column``` The NoteFieldColumn the layer is in.
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


# Making a noteskin for the NoteField
Read the comments in NoteSkins/default.
_fallback/Scripts/02 NoteSkin.lua has a couple minor functions for generating
state maps.

## 3D noteskins

Something about using a texture map instead of a state map.  The notes in
the default noteskin explain it.

# Noteskin Parameters

Noteskin parameters is a system for allowing a noteskin to be customized by the
player.  The parameters are defined by the noteskin and can do whatever the
skin author wants.  The player's choices are saved in their profile by skin
and stepstype (so the same skin can be set different ways for different
stepstypes).  The theme uses information provided by the noteskin to give the
player a menu for setting the parameters.

The system is explained in more detail in comments in the default noteskin,
with examples used to control explosion particles and the amount of warning
time given by the receptors.

Because the noteskin parameter table can go to any depth and contain anything,
and different players can choose different skins, creating a menu for it in
the confined OptionRow system would probably be very difficult.  OptionRow
menus for setting noteskin parameters are not provided.  The nested option
menus system does provide menus for setting noteskin parameters.


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
method is set by the NoteFieldColumn.

```NotePlayerizeMode_Off``` turns off playerizing of notes.  
```NotePlayerizeMode_Quanta``` turns of quantizing notes and instead uses
a different quanta for each player.  
```NotePlayerizeMode_Mask``` turns on a colored mask over the notes.  The
mask is a different color for each player.

If the theme attempts to set NotePlayerizeMode_Mask and the noteskin does not
support it, NotePlayerizeMode_Quanta will be used instead.


# Non-modifier Functions

Outdated until someone turns in a detailed review of the mod system
documentation.

## NoteFieldColumn Functions

* NoteFieldColumn:get_time_offset()  
Returns the ModifiableValue for offsetting the displayed note time (in
seconds) from the current time.

* NoteFieldColumn::get_quantization_parts_per_beat()  
Returns the ModifiableValue for changing the parts_per_beat value a note has
before it is used for quantization.  This can change 4ths into 8ths, or vice
versa.

This does not change the parts_per_beat that is passed to other modifiers.

* NoteFieldColumn::get_quantization_part_id()  
The part_id counterpart to get_quantization_parts_per_beat.

* NoteFieldColumn::get_speed_mod  
Returns the ModifiableValue for the speed mod.  This controls the y offset a
note has, which is its distance from the receptor.

* NoteFieldColumn::get_y_offset_vec_x()
* NoteFieldColumn::get_y_offset_vec_y()
* NoteFieldColumn::get_y_offset_vec_z()  
When positioning a note, the y offset value is multiplied by this 3D vector.
The vector defaults to {0, 1, 0}, which makes the notes approach from below.

* NoteFieldColumn::get_reverse_offset_pixels()  
This ModifiableValue controls the distance from the center of the notefield
to the receptor.  The player can configure the default value in their
notefield prefs.

* NoteFieldColumn::get_reverse_scale()  
ModifiableValue for controlling reverse.  1 makes the arrows scroll up, -1
makes the arrows scroll down.

* NoteFieldColumn::get_center_percent()  
ModifiableValue for controlling center.  Center kind of scales the
reverse_offset_pixels value.  When the center percent is 0, the receptor is
reverse_offset_pixels from the center of the field.  When the center percent
is 1, the receptor is at the center of the field.

* NoteFieldColumn::get_note_pos_x()
* NoteFieldColumn::get_note_pos_y()
* NoteFieldColumn::get_note_pos_z()
* NoteFieldColumn::get_note_rot_x()
* NoteFieldColumn::get_note_rot_y()
* NoteFieldColumn::get_note_rot_z()
* NoteFieldColumn::get_note_zoom_x()
* NoteFieldColumn::get_note_zoom_y()
* NoteFieldColumn::get_note_zoom_z()  
Functions for accessing the ModifiableValues used to position, rotate, and
scale each note.

* NoteFieldColumn::get_column_pos_x()
* NoteFieldColumn::get_column_pos_y()
* NoteFieldColumn::get_column_pos_z()
* NoteFieldColumn::get_column_rot_x()
* NoteFieldColumn::get_column_rot_y()
* NoteFieldColumn::get_column_rot_z()
* NoteFieldColumn::get_column_zoom_x()
* NoteFieldColumn::get_column_zoom_y()
* NoteFieldColumn::get_column_zoom_z()  
These ModifiableValues are used to position, rotate, and scale the column.
Because the column is an ActorFrame and the notes, receptors, and explosions
are actors rendered inside it.

* NoteFieldColumn::get_hold_normal_x()
* NoteFieldColumn::get_hold_normal_y()
* NoteFieldColumn::get_hold_normal_z()  
If the moddable hold normal flag is true, the result of this vector is
perpendicular to the surface of a hold.  This allows twisting holds without
applying a rotation to notes.  
When the flag is false, the normal is calculated from note positions.

* NoteFieldColumn::get_use_moddable_hold_normal()
* NoteFieldColumn::set_use_moddable_hold_normal(bool)  
Query or set the value of the moddable hold normal flag.

* NoteFieldColumn::get_note_alpha()
* NoteFieldColumn::get_note_glow()  
The alpha and glow values for notes are separately controllable by
ModifiableValues.

* NoteFieldColumn::get_receptor_alpha()
* NoteFieldColumn::get_receptor_glow()
* NoteFieldColumn::get_explosion_alpha()
* NoteFieldColumn::get_explosion_glow()  
The noteskin and theme can add layers to the column and set the fade type for
each layer.  A layer with the receptor fade type is affected by the receptor
alpha and glow ModifiableValues.  Similar for explosion fade type layers.

* NoteFieldColumn::get_use_game_music_beat()
* NoteFieldColumn::set_use_game_music_beat(bool)  
When the use game music flag is true (normal case), the column uses the
music time passed by its parent notefield.  When false, the column does not
update its music time, and must be updated by calling the set_curr_beat or
set_curr_second functions.

* NoteFieldColumn::get_show_unjudgable_notes()
* NoteFieldColumn::set_show_unjudgable_notes(bool)  
This flag is used to hide fake notes inside a warp when a cmod is used.

* NoteFieldColumn::get_speed_segments_enabled()
* NoteFieldColumn::set_speed_segments_enabled(bool)
* NoteFieldColumn::get_scroll_segments_enabled()
* NoteFieldColumn::set_scroll_segments_enabled(bool)  
These flags are used to turn off speed and scroll segments when a cmod is
used.

* NoteFieldColumn::get_holds_skewed_by_mods()
* NoteFieldColumn::set_holds_skewed_by_mods(bool)  
If holds are skewed by mods, then mods like drunk make the hold look thinner
in places because the verts on the left and right edges have the same y value
as the corresponding point in the center.  
When this flag is false, the engine calculates a forward vector and arranges
the left and right verts to be perpendicular to that.  This makes drunk curve
holds.

* NoteFieldColumn::get_twirl_holds()
* NoteFieldColumn::set_twirl_holds(bool)  
If this flag is falce, y rotation of notes does not affect holds.

* NoteFieldColumn::get_playerize_mode()
* NoteFieldColumn::set_playerize_mode(NotePlayerizeMode)  
NotePlayerizeMode enum for controlling how notes are playerized in couples
mode.  NotePlayerizeMode_Mask requires support in the noteskin, so it will
probably not work.

* NoteFieldColumn::get_curr_beat()
* NoteFieldColumn::set_curr_beat(float)
* NoteFieldColumn::get_curr_second()
* NoteFieldColumn::set_curr_second(float)  
Functions for getting and setting the time the column uses to render.

* NoteFieldColumn::set_pixels_visible_before(float)
* NoteFieldColumn::set_pixels_visible_after(float)  
The pixels visible values are actually y offset values.  A note with a y offset value in this range is rendered, with adjustments for reverse.

* NoteFieldColumn::get_upcoming_time()
* NoteFieldColumn::set_upcoming_time(float)  
Column layers are passed a time to the next upcoming note.  This value
controls how far ahead the column looks for a note.

* NoteFieldColumn::get_layer_fade_type(layer)
* NoteFieldColumn::set_layer_fade_type(layer, FieldLayerFadeType)  
layer must be an actor that is a child of the column.  
This function is how receptors and explosions in the noteskin tell the column
what glow and alpha mods to apply to the layer.  
Type names: ```FieldLayerFadeType_Receptor```, ```FieldLayerFadeType_Note```, ```FieldLayerFadeType_Explosion```, ```FieldLayerFadeType_None```.

* NoteFieldColumn::get_layer_transform_type(layer)
* NoteFieldColumn::set_layer_transform_type(layer, FieldLayerTransformType)  
layer must be an actor that is a child of the column.  
Any child of the column (such as the receptor) defaults to
FieldLayerTransformType_Full.  FieldLayerTransformType_PosOnly disables the
rotation from note mods.  FieldLayerTransformType_None disables the
positioning and rotation from note mods.
Type names: ```FieldLayerTransformType_Full```, ```FieldLayerTransformType_PosOnly```, ```FieldLayerTransformType_None```

### Why do these exist functions
These functions do not serve their purpose well or reliable and are subject
to change whenever I get a better idea.

* NoteFieldColumn::receptor_y_offset()  
Returns the y offset given to receptors.  Usually 0 unless the speed mod
returns non-zero for a time distance of 0.

* NoteFieldColumn::get_reverse_shift()  
Returns the current reverse shift value.  I don't know how this could be
useful, probably just needed it for debugging one day.

* NoteFieldColumn::apply_column_mods_to_actor(actor)  
Use the column's ModifiableValues to position the actor.

* NoteFieldColumn::apply_note_mods_to_actor(actor, bool time_is_offset, float beat, float second, float y_offset, bool use_alpha, bool use_glow)  
Applies note modifiers to the actor as if it were a note inside the column.
This will not position the actor perfectly to move with the notes unless the
actor is a child of the column.  Even then it probably won't be positioned
right.



## NoteField Functions

* NoteField:get_columns()  
Returns a table of the NoteFieldColumns in the NoteField.

* NoteField:get_width()  
The width of each column, and thus the field, is controlled by the noteskin.
get_width() returns the total width of all columns and their padding.

* NoteField:get_curr_beat()
* NoteField:set_curr_beat(float)
* NoteField:get_curr_second()
* NoteField:set_curr_second(float)  
Normally, the NoteField is given a current time by the Player it is for, and
that is passed to each column.  
These functions allow setting the current time instead of using the time from
the Player.

* NoteField:get_trans_pos_x()
* NoteField:get_trans_pos_y()
* NoteField:get_trans_pos_z()
* NoteField:get_trans_rot_x()
* NoteField:get_trans_rot_y()
* NoteField:get_trans_rot_z()
* NoteField:get_trans_zoom_x()
* NoteField:get_trans_zoom_y()
* NoteField:get_trans_zoom_z()  
ModifiableValues for controlling the position of the NoteField.  
The player has a menu for configuring the values they prefer.

* NoteField:get_receptor_alpha()
* NoteField:get_receptor_glow()
* NoteField:get_explosion_alpha()
* NoteField:get_explosion_glow()  
The NoteField doesn't have receptor or explosion layers unless the theme adds
layers and sets their fade type.  But if those layers exist, these
ModifiableValues set their alpha and glow.

* NoteField:get_fov_mod()  
For when you want a mod that changes the FOV.

* NoteField:get_vanish_x_mod()
* NoteField:get_vanish_y_mod()
* NoteField:get_vanish_type()
* NoteField:set_vanish_type(FieldVanishType)  
Incoming and Space perspectives in the old system worked in part by moving
the vanish point towards the center of the screen.  
These functions allow controlling the vanish point with mods.  
Since the vanish point used to calculate the projection matrix has to be a
point on the screen, this is not simple.  
With FieldVanishType_RelativeToParent (the default), the Player and NoteField
positions are added to the result from the mods.  
With FieldVanishType_RelativeToSelf, only the NoteField position is added.  
With FieldVanishType_RelativeToOrigin, the mod result is used raw.  
I'm not entirely satisfied with this system because it's not really right if
the Player rotates, and other problems when the NoteField isn't attached to a
Player.

* NoteField:set_player_color(int, color)  
In the unlikely event that a noteskin provides masks and a chart for multiple
players is being played in NotePlayerizeMode_Mask, this function can be used
to set what color each player's notes are masked with.

* NoteField:get_layer_fade_type()
* NoteField:set_layer_fade_type(layer, type)  
Similar to the NoteFieldColumn functions of the same name.  Layers in the
NoteField can control which alpha and glow mods affect them.

### Compatibility functions
For compatibility with gimmick files made for 5.0.

* NoteField:get_defective_mode()
* NoteField:set_defective_mode(bool)  
Turning on defective mode disables all the ModifiableValue fields in
NoteFieldColumn and NoteField to use the modifier values in PlayerOptions
instead.  
There are a variety of checks to detect something using PlayerOptions and set
this flag to true automatically.

* NoteField:get_oitg_zoom_mode()
* NoteField:set_oitg_zoom_mode(bool)  
OITG has a bug where Actor:SetZoom does not set the z zoom.  This affects how
Mini (which sets notefield zoom) and Bumpy (which sets note z position)
interact.  
Any gimmick simfile ported from oitg needs this flag set to true.  
The oitg_zoom_mode_actor() function provides an easy to use wrapper for
setting this flag for all players.


### Arcana functions
These functions are meant for engine internal use.  Using them right usually
requires knowledge of other parts of the engine.

* NoteField:get_skin()
* NoteField:set_skin(name, params)  
Setting the noteskin should be left up to the code in the engine that fetches
the right params from the profile.  
Loading the noteskin is slow, if you change the noteskin during gameplay, the
notes will hitch.

* NoteField:set_steps(steps)  
Change the Steps being displayed by the notefield.  This will cause the game
to hitch while the note data is loaded from the simfile.


### Functions provided by _fallback

Kyz took another break.

* NoteField:set_speed_mod(constant, speed, read_bpm)  
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

* NoteField:set_rev_offset_base(revoff)  
Sets the base value for reverse_offset_pixels for all columns to revoff.

* NoteField:set_reverse_base(rev)  
Sets the base value for reverse_scale for all columns to rev.

* NoteField:clear_column_mod(mod_field_name, mod_name)  
Clears the mod with mod_name in the field mod_field_name in all columns.

* NoteField:set_hidden_mod(line, dist, add_glow)  
Adds a hidden mod, which makes notes disappear as they cross the line.  line
is the y offset the mod effect is occurs at.  dist is how many pixels notes
take to go from fully visible to fully invisible.  If add_glow is true, then
the glow on notes will ramp up before they fade out.  If add_glow is false,
then the hidden mod will only make the notes transparent to fade them out.

* NoteField:set_sudden_mod(line, dist, add_glow)  
Similar to set_hidden_mod, this adds a sudden mod, which makes notes after
line invisible, and notes above it visible.

* NoteField:clear_hidden_mod()  
Clears the hidden mod from all columns.

* NoteField:clear_sudden_mod()  
Clears the sudden mod from all columns.

## Other functions

* find_pactor_in_gameplay(screen_gameplay, pn)  
Returns the player actor or nil.

* find_notefield_in_gameplay(screen_gameplay, pn)  
Returns the notefield actor or nil.

* notefield_prefs_actor()
Returns an actor that applies notefield prefs for both players.

* noteskin_option_row()  
Returns a lua option row for picking a noteskin on a metrics based options
screen.
