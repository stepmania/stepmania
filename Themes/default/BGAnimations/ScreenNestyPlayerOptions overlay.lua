local menu_x= {
	[PLAYER_1]= _screen.w * .25,
	[PLAYER_2]= _screen.w * .75,
}
local menus= {}
for i, pn in ipairs(GAMESTATE:GetHumanPlayers()) do
	menus[pn]= setmetatable({}, nesty_menu_stack_mt)
end

local notefield_config= {
	nesty_options.float_config_val(newfield_prefs_config, "hidden_offset", -1, 1, 2),
	nesty_options.float_config_val(newfield_prefs_config, "sudden_offset", -1, 1, 2),
	nesty_options.bool_config_val(newfield_prefs_config, "hidden"),
	nesty_options.bool_config_val(newfield_prefs_config, "sudden"),
	nesty_options.float_config_val(newfield_prefs_config, "fade_dist", -1, 1, 2),
	nesty_options.bool_config_val(newfield_prefs_config, "glow_during_fade"),
	nesty_options.float_config_val(newfield_prefs_config, "fov", -1, 0, 1, 1, 179),
	nesty_options.float_config_val(newfield_prefs_config, "reverse", -2, 0, 0),
	nesty_options.float_config_val(newfield_prefs_config, "rotation_x", -1, 1, 2),
	nesty_options.float_config_val(newfield_prefs_config, "rotation_y", -1, 1, 2),
	nesty_options.float_config_val(newfield_prefs_config, "rotation_z", -1, 1, 2),
	nesty_options.float_config_val(newfield_prefs_config, "vanish_x", -1, 1, 2),
	nesty_options.float_config_val(newfield_prefs_config, "vanish_y", -1, 1, 2),
	nesty_options.float_config_val(newfield_prefs_config, "yoffset", -1, 1, 2),
	nesty_options.float_config_val(newfield_prefs_config, "zoom", -2, -1, 1),
	nesty_options.float_config_val(newfield_prefs_config, "zoom_x", -2, -1, 1),
	nesty_options.float_config_val(newfield_prefs_config, "zoom_y", -2, -1, 1),
	nesty_options.float_config_val(newfield_prefs_config, "zoom_z", -2, -1, 1),
}

local function gen_speed_menu(pn)
	local prefs= newfield_prefs_config:get_data(pn)
	if prefs.speed_type == "multiple" then
		return nesty_options.float_config_val_args(newfield_prefs_config, "speed_mod", -2, -1, 1)
	else
		return nesty_options.float_config_val_args(newfield_prefs_config, "speed_mod", -2, 1, 3)
	end
end

local turn_chart_mods= {
	nesty_options.bool_player_mod_val("Mirror"),
	nesty_options.bool_player_mod_val("Backwards"),
	nesty_options.bool_player_mod_val("Left"),
	nesty_options.bool_player_mod_val("Right"),
	nesty_options.bool_player_mod_val("Shuffle"),
	nesty_options.bool_player_mod_val("SoftShuffle"),
	nesty_options.bool_player_mod_val("SuperShuffle"),
}

local removal_chart_mods= {
	nesty_options.bool_player_mod_val("NoHolds"),
	nesty_options.bool_player_mod_val("NoRolls"),
	nesty_options.bool_player_mod_val("NoMines"),
	nesty_options.bool_player_mod_val("HoldRolls"),
	nesty_options.bool_player_mod_val("NoJumps"),
	nesty_options.bool_player_mod_val("NoHands"),
	nesty_options.bool_player_mod_val("NoLifts"),
	nesty_options.bool_player_mod_val("NoFakes"),
	nesty_options.bool_player_mod_val("NoQuads"),
	nesty_options.bool_player_mod_val("NoStretch"),
}

local insertion_chart_mods= {
	nesty_options.bool_player_mod_val("Little"),
	nesty_options.bool_player_mod_val("Wide"),
	nesty_options.bool_player_mod_val("Big"),
	nesty_options.bool_player_mod_val("Quick"),
	nesty_options.bool_player_mod_val("BMRize"),
	nesty_options.bool_player_mod_val("Skippy"),
	nesty_options.bool_player_mod_val("Mines"),
	nesty_options.bool_player_mod_val("AttackMines"),
	nesty_options.bool_player_mod_val("Echo"),
	nesty_options.bool_player_mod_val("Stomp"),
	nesty_options.bool_player_mod_val("Planted"),
	nesty_options.bool_player_mod_val("Floored"),
	nesty_options.bool_player_mod_val("Twister"),
}

local chart_mods= {
	{name= "turn_chart_mods", meta= nesty_option_menus.menu,
	 translatable= true, args= turn_chart_mods},
	{name= "removal_chart_mods", meta= nesty_option_menus.menu,
	 translatable= true, args= removal_chart_mods},
	{name= "insertion_chart_mods", meta= nesty_option_menus.menu,
	 translatable= true, args= insertion_chart_mods},
}

local base_options= {
	{name= "speed_mod", meta= nesty_option_menus.adjustable_float,
	 translatable= true, args= gen_speed_menu, exec_args= true},
	{name= "speed_type", meta= nesty_option_menus.enum_option,
	 translatable= true,
	 args= {
		 name= "speed_type", enum= newfield_speed_types, fake_enum= true,
		 obj_get= function(pn) return newfield_prefs_config:get_data(pn) end,
		 get= function(obj) return obj.speed_type end,
		 set= function(obj, value)
			 if obj.speed_type == "multiple" and value ~= "multiple" then
				 obj.speed_mod= math.round(obj.speed_mod * 100)
			 elseif obj.speed_type ~= "multiple" and value == "multiple" then
				 obj.speed_mod= obj.speed_mod / 100
			 end
			 obj.speed_type= value
		 end,
	}},
	nesty_options.float_song_mod_val("MusicRate", -2, -1, -1, .5, 2),
	nesty_options.float_song_mod_toggle_val("Haste", 1, 0),
	nesty_options.float_config_toggle_val(newfield_prefs_config, "reverse", -1, 1),
	nesty_options.float_config_val(newfield_prefs_config, "zoom", -2, -1, 1),
	{name= "chart_mods", translatable= true, meta= nesty_option_menus.menu, args= chart_mods},
	{name= "newskin", translatable= true, meta= nesty_option_menus.newskins},
	{name= "newskin_params", translatable= true, meta= nesty_option_menus.menu,
	 args= gen_noteskin_param_menu, req_func= show_noteskin_param_menu},
	nesty_options.bool_config_val(newfield_prefs_config, "hidden"),
	nesty_options.bool_config_val(newfield_prefs_config, "sudden"),
	{name= "advanced_notefield_config", translatable= true, meta= nesty_option_menus.menu, args= notefield_config},
	{name= "reload_newskins", translatable= true, meta= "execute",
	 execute= function() NEWSKIN:reload_skins() end},
}

local player_ready= {}

local function exit_if_both_ready()
	for i, pn in ipairs(GAMESTATE:GetHumanPlayers()) do
		if not player_ready[pn] then return end
	end
	SCREENMAN:GetTopScreen():StartTransitioningScreen("SM_GoToNextScreen")
end

local function input(event)
	if menu_stack_generic_input(menus, event) then
		player_ready[event.PlayerNumber]= true
		exit_if_both_ready()
	end
end

local frame= Def.ActorFrame{
	OnCommand= function(self)
		SCREENMAN:GetTopScreen():AddInputCallback(input)
		for pn, menu in pairs(menus) do
			menu:push_options_set_stack(nesty_option_menus.menu, base_options, "Play Song")
			menu:update_cursor_pos()
		end
	end,
}
for pn, menu in pairs(menus) do
	frame[#frame+1]= menu:create_actors{
		x= menu_x[pn], y= 56, width= _screen.w * .5, height= _screen.h * .75,
		num_displays= 1, pn= pn,
		el_height= 24,
	}
	menu:set_translation_section("newfield_options")
end

return frame
