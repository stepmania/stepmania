# Querying the system
```lua
local input_list= ModValue.get_input_list()
-- Check whether music_rate exists
if input_list.music_rate then
...
end
-- Print the list to the log file for reading.
foreach_ordered(input_list, function(name, exists) do Trace(name) end)
```

# Input Descriptions

```'music_rate'```
The current music rate, adjusted for any haste effect.  The M and C speed
mods provided by _fallback use this to adjust for rate mods.

```'column'```
The 0-indexed id of the column the equation is evaluated for.  This is zero
for the notefield.

```'y_offset'```
The result of the speed modifier.  This is zero when the speed modifier is
evaluated, so don't use it in speed mods and expect a useful result.

```'note_id_in_chart'```
note_id_in_chart starts at zero and counts all notes in the chart up to the
current note.

```'note_id_in_column'```
note_id_in_column starts at zero and counts all notes in the column up to the
current note.

```'row_id'```
row_id starts at zero and counts up for every row that has notes.  row_id is
not the beat number, row_id is not calculated from the beat number.

Picture explanation:
http://i.imgur.com/x0RqDbZ.png

Note and row ids are meaningless for mods that are not evaluated for every
note.  Per-note mods (targets) are: speed, y_offset_vec, note_* (pos, rot,
zoom, alpha, glow), hold_normal_*, quantization_*.

For hold notes, note and row ids are the id for the note that started the
hold, not the note or row id for the current time.

```'eval_beat'```
```'eval_second'```
The time of the current note, in beats or seconds.  This will be equal to the
music time if it is not a per-note mod.

```'music_beat'```
```'music_second'```
The current time for the receptors, in beats or seconds.

```'dist_beat'```
```'dist_second'```
eval time minus music time.

```'start_beat'```
```'start_second'```
The start time for the modifier this equation is inside.  Thus, you do not
have to change the start beat everywhere inside the equation when adjusting
modifier.

```'length_beats'```
```'length_seconds'```
Length as a an input field, similar to start.

```'end_beat'```
```'end_second'```
end as a an input field, similar to start.

```'prefunres'```
The result of previous equations as an input field.
