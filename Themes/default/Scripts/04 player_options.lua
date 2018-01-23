local chart_mod_names= {
	{"turn", {
		 "Mirror",
		 "Backwards",
		 "Left",
		 "Right",
		 "Shuffle",
		 "SoftShuffle",
		 "SuperShuffle",
	}},
	{"removal", {
		 "NoHolds",
		 "NoRolls",
		 "NoMines",
		 "HoldRolls",
		 "NoJumps",
		 "NoHands",
		 "NoLifts",
		 "NoFakes",
		 "NoQuads",
		 "NoStretch",
	}},
	{"insertion", {
		 "Little",
		 "Wide",
		 "Big",
		 "Quick",
		 "BMRize",
		 "Skippy",
		 "Mines",
		 "Echo",
		 "Stomp",
		 "Planted",
		 "Floored",
		 "Twister",
	}},
}

local chart_mods= {}
for tid, info in ipairs(chart_mod_names) do
	local items= {}
	for nid, name in ipairs(info[2]) do
		items[#items+1]= {"item", "player_option", name}
	end
	chart_mods[#chart_mods+1]= {"submenu", info[1].."_chart_mods", items}
end

local gameplay_options= {}
for i, name in ipairs{"ComboUnderField", "FlashyCombo", "GameplayShowScore", "JudgmentUnderField", "Protiming"} do
	gameplay_options[#gameplay_options+1]= {"item", player_config, name, "bool"}
end

local life_options= {
	{"item", "player_option", "LifeSetting"},
	{"item", "player_option", "DrainSetting"},
	{"item", "player_option", "FailSetting"},
	{"item", "player_option", "BatteryLives"},
}

local song_options= {
	{"item", "song_option", "StaticBackground", "bool"},
	{"item", "song_option", "RandomBGOnly", "bool"},	
	{"item", "song_option", "AssistClap", "bool"},
	{"item", "song_option", "AssistMetronome", "bool"},
	{"item", "song_option", "MusicRate"},
	{"item", "song_option", "Haste"},	
}

local menu_data= {
	-- Play and back are inserted lower in this file.
	notefield_prefs_speed_type_item(),
	notefield_prefs_speed_mod_item(),	
	notefield_prefs_perspective_item(),
	{"item", notefield_prefs_config, "reverse", "toggle_number", {on= -1, off= 1}},
	{"item", notefield_prefs_config, "zoom", "percent"},
	{"submenu", "chart_mods", chart_mods},
	{"item", notefield_prefs_config, "hidden", "bool"},
	{"item", notefield_prefs_config, "sudden", "bool"},
	adv_notefield_prefs_menu(),
	get_notefield_mods_toggle_menu(true, false),
	-- Uncomment this for more advanced effects mods.
	-- get_notefield_mods_value_menu(true, false),
	noteskin_menu_item(),
	noteskin_params_menu_item(),
	shown_noteskins_menu(),
	{"item", player_config, "ScreenFilter", "percent"},	
	{"submenu", "song_options", song_options},	
	{"submenu", "life_options", life_options},
	{"submenu", "gameplay_options", gameplay_options},
}

function get_player_options_menu()
	-- Return a shallow copy of the menu so that a screen can add items to it
	-- without causing duplication problems. -Kyz
	local ret= {}
	for i, item in ipairs(menu_data) do
		ret[#ret+1]= item
	end
	return ret
end
