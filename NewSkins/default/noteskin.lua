return {
	-- notes is the file to use to load the note columns, the taps and holds.
	notes= "notes.lua",
	-- layers_below_notes is a list of files to load things in the columns that
	-- are displayed underneath the notes.  Typically, this is just the
	-- receptors.  The layers are rendered in the order they are in the table,
	-- so the first layer is on the bottom.
	-- receptors.lua has the explanation of the requirements of a layer file.
	layers_below_notes= {"receptors.lua"},
	-- layers_above_notes is the same as layers_below_notes, but its contents
	-- are rendered after the notes, so they appear on top of the notes.
	layers_above_notes= {"explosions.lua"},
	-- Since all layers are considered the same, messages such as
	-- judgment and step actions are sent to all layers.  This means you can
	-- make receptors that respond to judgments, or explosions that respond to
	-- steps, or whatever.
	supports_all_buttons= false,
	buttons= {"Left", "Down", "Up", "Right"},
	-- The fallback entry is optional.  It can be used to name another noteskin
	-- to fall back on if a hold texture or one of the files listed above is
	-- not found.
	fallback= "",
}
