# What is NewField?
NewField is a complete replacement for the NoteField, NoteSkin, and modifier
systems, written from scratch.  The new systems are designed to be more
flexible and predictable and documented on the user side, and easier to
maintain on the developer side.

Due to the completely new design, nothing is compatible.


# Making a theme use NewField
For transition period builds, both the old NoteField and the NewField exist
on ScreenGameplay.  The NewField is disabled unless the theme calls special
functions to enable it, and a NewSkin cannot be chosen without being added to
the metrics.

## Enabling
To enable the NewField on ScreenGameplay, the function
use_newfield_on_gameplay should be called during an OnCommand on
ScreenGameplay.  For convenience, an actor that does this is packaged in the
use_newfield_actor function.  The number passed in is the distance from the
center of the field to the receptors (aka: reverse offset).  
Example: ```t[#t+1]= use_newfield_actor(144)```  
The speed, perspective, and mini mods will be read from the old
PlayerOptions structure and converted to the new mod system.  Any other
modifiers that affect how notes are displayed are ignored for now.

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


# Multiplayer mode
(aka: routine mode)
Playerizing a note means doing something to make it look unique for each
player.  It is used in multiplayer (routine) mode to show which notes are
for player 1 and which are for player 2.

Playerizing is still under active development and open for new ideas.  An
ideal playerizing method should make it possible for notes to be playerized
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
