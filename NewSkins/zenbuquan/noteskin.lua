-- The zenbuquan noteskin supports all buttons so that if a player picks a
-- stepstype they don't have a noteskin for, they'll see *something*.
return {
	notes= "notes.lua",
	layers_below_notes= {"receptors.lua"},
	layers_above_notes= {"explosions.lua"},
	supports_all_buttons= true,
}
