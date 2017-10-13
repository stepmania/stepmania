# Querying the system
If you have a NoteField or NoteFieldColumn actor, the get_mod_target_info
function of that actor will return a list of target names.

The functions for getting a list of targets must be called on a NoteField or
NoteFieldColumn actor.

load_notefield_mods_file returned a table containing the notefields, so that
is used in this example.

```lua
local notefield_targets= notefields[PLAYER_1]:get_mod_target_info()
local column_targets= notefields[PLAYER_1]:get_columns()[1]:get_mod_target_info()
-- Check whether transform_rot_z exists
if notefield_targets.transform_rot_z then
	...
end
-- Print the list to the log file for reading.
foreach_ordered(notefield_targets, function(name, exists) do Trace(name) end)
```

# Target details

The target info table has this structure (and these are the names of all
targets, unless this document is outdated):
```lua
-- NoteField target info
{
	transform_pos_x= true,
	transform_pos_y= true,
	transform_pos_z= true,
	transform_rot_x= true,
	transform_rot_y= true,
	transform_rot_z= true,
	transform_zoom= true,
	transform_zoom_x= true,
	transform_zoom_y= true,
	transform_zoom_z= true,
	receptor_alpha= true,
	receptor_glow= true,
	explosion_alpha= true,
	explosion_glow= true,
	fov_x= true,
	fov_y= true,
	fov_z= true,
}
```
transform values move, rotate, or zoom the field as a whole.

rotation results are multiplied by pi internally, so setting transform_rot_z
to 1 rotates halfway around the z axis.

zoom_x, zoom_y, and zoom_z are multiplied by zoom to get the final zoom.

receptor_alpha and receptor_glow affect layers in the theme's
```Graphics/NoteField layers.lua``` that are marked as receptor layers.

explosion_alpha and explosion_glow affect layers in the theme's
```Graphics/NoteField layers.lua``` that are marked as explosion layers.

fov_x and fov_y change the position of the vanish point, identical to
ActorFrame's vanishpoint function.

fov_z changes the field of view angle.

```lua
-- NoteFieldColumn target info
{
	column_pos_x= true,
	column_pos_y= true,
	column_pos_z= true,
	column_rot_x= true,
	column_rot_y= true,
	column_rot_z= true,
	column_zoom= true,
	column_zoom_x= true,
	column_zoom_y= true,
	column_zoom_z= true,
	note_pos_x= true,
	note_pos_y= true,
	note_pos_z= true,
	note_rot_x= true,
	note_rot_y= true,
	note_rot_z= true,
	note_zoom= true,
	note_zoom_x= true,
	note_zoom_y= true,
	note_zoom_z= true,
	speed= true,
	y_offset_vec_x= true,
	y_offset_vec_y= true,
	y_offset_vec_z= true,
	reverse_offset= true,
	reverse= true,
	center= true,
	hold_normal_x= true,
	hold_normal_y= true,
	hold_normal_z= true,
	explosion_alpha= true,
	explosion_glow= true,
	note_alpha= true,
	note_glow= true,
	receptor_alpha= true,
	receptor_glow= true,
	time_offset= true,
	-- deprecated
	lift_pretrail= true,
	quantization_multiplier= true,
	quantization_offset= true,
}
```

column values move the column as a whole.  The column is an ActorFrame, and
notes are rendered as children, so moving the column also moves all notes in
it.  The theme also has layers in ```Graphics/NoteFieldColumn layers.lua```
that are also tied to the column position.  Receptors and explosions from the
noteskin are also layers, identical in behavior to the theme's layers.

note values move individual notes.

zoom_x, zoom_y, and zoom_z are multiplied by zoom to get the final zoom.

speed is used to calculate the y_offset of each note.  Typically it just uses
the eval_beat and music_beat multiplied by some constant.

y_offset_vec is used to apply the y_offset of a note to the note's final
position.  This defaults to {0, 1, 0} (point towards positive y).
```(y_offset_vec * y_offset) + note_pos``` is the note's final position,
relative to the receptors, ignoring reverse and center effects.

reverse_offset is the distance in pixels from the center of the notefield to
the receptors, before reverse and center are applied.  Zero puts the
receptors in the center.

reverse both scales the note positions vertically, and reverses the direction
notes travel.  reverse defaults to 1 (scrolling down), and -1 makes notes
scroll up.

Noteskin and theme column layers are informed when reverse changes from
negative to positive or back.

center shifts the receptors closer to the notefield.  0 puts the receptors
reverse_offset pixels from the center, 1 puts the receptors in the center,
2, puts the receptors reverse_offset pixels below the center.

hold_normal is for when the hold body needs to twist in its own way instead
of using the note rotation.  This is a normal vector, it points perpendicular
to the surface of the hold.  Use set_use_moddable_hold_normal when using
hold_normal mods.

explosion and receptor alpha and glow affect the layers the noteskin and
theme mark as being explosions or receptors.  note alpha and glow apply alpha
and glow to notes.

time_offset makes the column display a different time from the rest of the
notefield.  Someone wanted per-player time offset for their beatmania mode
and it somehow ended up per-column.

#### Deprecated

lift_pretrail affects how many beats or seconds ahead of the lift its
attached hold body will start to show up.  This should not be a modifier, it
should be part of the note data.

quantization_multiplier and quantization_offset affect the value that is used
to quantize the note and decide whether it is a 4th, an 8th, or some other
quantization.  The quantization system isn't satisfactory and should be
overhauled for better quantization and time signature support.
