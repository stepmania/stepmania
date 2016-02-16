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


## Notefield board
The notefield board is not stretched by the NewField.  (the old notefield had
some complex code that would stretch the board to the height of the field if
it was an image)
The NewField sends more info to the notefield board to make sizing simpler.
The info is sent through commands that are executed on the board.  Note that
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
