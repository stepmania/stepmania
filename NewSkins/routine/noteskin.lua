return {
	-- See explanation comments in default/noteskin.lua.
	notes= "notes.lua",
	layers_below_notes= {"receptors.lua"},
	layers_above_notes= {"explosions.lua"},
	-- These are example player colors for multiplayer mask mode.
	-- Any number of player colors can be specified and the notefield will use
	-- the ones needed.
	player_colors= {{1, 0, 0, 1}, {0, 1, 0, 1}, {0, 0, 1, 1},
		{0, 1, 1, 1}, {1, 0, 1, 1}, {1, 1, 0, 1}},
	supports_all_buttons= false,
	buttons= {"Left", "Down", "Up", "Right"},
	fallback= "default",
}
