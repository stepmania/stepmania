local menu_height= 300
local menu_width= 250
local menu_x= {
	[PLAYER_1]= _screen.w * .25,
	[PLAYER_2]= _screen.w * .75,
}
local menus= {}
for i, pn in ipairs(GAMESTATE:GetHumanPlayers()) do
	menus[pn]= setmetatable({}, nesty_menu_stack_mt)
end
local explanations= {}
local ready_indicators= {}

local notefield_config= {
	nesty_options.float_config_val(newfield_prefs_config, "hidden_offset", -1, 1, 2),
	nesty_options.float_config_val(newfield_prefs_config, "sudden_offset", -1, 1, 2),
	nesty_options.bool_config_val(newfield_prefs_config, "hidden"),
	nesty_options.bool_config_val(newfield_prefs_config, "sudden"),
	nesty_options.float_config_val(newfield_prefs_config, "fade_dist", -1, 1, 2),
	nesty_options.bool_config_val(newfield_prefs_config, "glow_during_fade"),
	nesty_options.float_config_val(newfield_prefs_config, "reverse", -2, 0, 0),
	nesty_options.float_config_val(newfield_prefs_config, "zoom", -2, -1, 1),
	nesty_options.float_config_val(newfield_prefs_config, "rotation_x", -1, 1, 2),
	nesty_options.float_config_val(newfield_prefs_config, "rotation_y", -1, 1, 2),
	nesty_options.float_config_val(newfield_prefs_config, "rotation_z", -1, 1, 2),
	nesty_options.float_config_val(newfield_prefs_config, "vanish_x", -1, 1, 2),
	nesty_options.float_config_val(newfield_prefs_config, "vanish_y", -1, 1, 2),
	nesty_options.float_config_val(newfield_prefs_config, "fov", -1, 0, 1, 1, 179),
	nesty_options.float_config_val(newfield_prefs_config, "yoffset", -1, 1, 2),
	nesty_options.float_config_val(newfield_prefs_config, "zoom_x", -2, -1, 1),
	nesty_options.float_config_val(newfield_prefs_config, "zoom_y", -2, -1, 1),
	nesty_options.float_config_val(newfield_prefs_config, "zoom_z", -2, -1, 1),
}

local function gen_speed_menu(pn)
	local prefs= newfield_prefs_config:get_data(pn)
	if prefs.speed_type == "multiple" then
		return nesty_options.float_config_val_args(newfield_prefs_config, "speed_mod", -2, -1, 1)
	else
		return nesty_options.float_config_val_args(newfield_prefs_config, "speed_mod", 0, 1, 3)
	end
end

local function trisign_of_num(num)
	if num < 0 then return -1 end
	if num > 0 then return 1 end
	return 0
end

-- Skew needs to shift towards the center of the screen.
local pn_skew_mult= {[PLAYER_1]= 1, [PLAYER_2]= -1}

local function perspective_entry(name, skew_mult, rot_mult)
	return {
		name= name, translatable= true, type= "choice", execute= function(pn)
			local conf_data= newfield_prefs_config:get_data(pn)
			local old_rot= get_element_by_path(conf_data, "rotation_x")
			local old_skew= get_element_by_path(conf_data, "vanish_x")
			local new_rot= rot_mult * 30
			local new_skew= skew_mult * 160 * pn_skew_mult[pn]
			set_element_by_path(conf_data, "rotation_x", new_rot)
			set_element_by_path(conf_data, "vanish_x", new_skew)
			-- Adjust the y offset to make the receptors appear at the same final
			-- position on the screen.
			if new_rot < 0 then
				set_element_by_path(conf_data, "yoffset", 180)
			elseif new_rot > 0 then
				set_element_by_path(conf_data, "yoffset", 140)
			else
				set_element_by_path(conf_data, "yoffset", get_element_by_path(newfield_prefs_config:get_default(), "yoffset"))
			end
			MESSAGEMAN:Broadcast("ConfigValueChanged", {
				config_name= newfield_prefs_config.name, field_name= "rotation_x", value= new_rot, pn= pn})
		end,
		value= function(pn)
			local conf_data= newfield_prefs_config:get_data(pn)
			local old_rot= get_element_by_path(conf_data, "rotation_x")
			local old_skew= get_element_by_path(conf_data, "vanish_x")
			if trisign_of_num(old_rot) == trisign_of_num(rot_mult) and
			trisign_of_num(old_skew) == trisign_of_num(skew_mult) * pn_skew_mult[pn] then
				return true
			end
			return false
		end,
	}
end

local perspective_mods= {
	perspective_entry("overhead", 0, 0),
	perspective_entry("distant", 0, -1),
	perspective_entry("hallway", 0, 1),
	perspective_entry("incoming", 1, -1),
	perspective_entry("space", 1, 1),
}

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
	nesty_options.bool_player_mod_val("Echo"),
	nesty_options.bool_player_mod_val("Stomp"),
	nesty_options.bool_player_mod_val("Planted"),
	nesty_options.bool_player_mod_val("Floored"),
	nesty_options.bool_player_mod_val("Twister"),
}

local chart_mods= {
	{name= "turn_chart_mods", menu= nesty_option_menus.menu,
	 translatable= true, args= turn_chart_mods},
	{name= "removal_chart_mods", menu= nesty_option_menus.menu,
	 translatable= true, args= removal_chart_mods},
	{name= "insertion_chart_mods", menu= nesty_option_menus.menu,
	 translatable= true, args= insertion_chart_mods},
}

local gameplay_options= {
	nesty_options.bool_config_val(player_config, "ComboUnderField"),
	nesty_options.bool_config_val(player_config, "FlashyCombo"),
	nesty_options.bool_config_val(player_config, "GameplayShowStepsDisplay"),
	nesty_options.bool_config_val(player_config, "GameplayShowScore"),
	nesty_options.bool_config_val(player_config, "JudgmentUnderField"),
}

-- The time life bar doesn't work sensibly outside the survival courses, so
-- keep it out of the menu.
local life_type_enum= {"LifeType_Bar", "LifeType_Battery"}
local life_options= {
	nesty_options.enum_player_mod_single_val("bar_type", "LifeType_Bar", "LifeSetting"),
	nesty_options.enum_player_mod_single_val("battery_type", "LifeType_Battery", "LifeSetting"),
	nesty_options.enum_player_mod_single_val("normal_drain", "DrainType_Normal", "DrainSetting"),
	nesty_options.enum_player_mod_single_val("no_recover", "DrainType_NoRecover", "DrainSetting"),
	nesty_options.enum_player_mod_single_val("sudden_death", "DrainType_SuddenDeath", "DrainSetting"),
	nesty_options.enum_player_mod_single_val("fail_immediate", "FailType_Immediate", "FailSetting"),
	nesty_options.enum_player_mod_single_val("fail_immediate_continue", "FailType_ImmediateContinue", "FailSetting"),
	nesty_options.enum_player_mod_single_val("fail_end_of_song", "FailType_EndOfSong", "FailSetting"),
	nesty_options.enum_player_mod_single_val("fail_off", "FailType_Off", "FailSetting"),
	nesty_options.float_player_mod_val("BatteryLives", 0, 0, 0, 1, 10, 4),
}

local base_options= {
	{name= "speed_mod", menu= nesty_option_menus.adjustable_float,
	 translatable= true, args= gen_speed_menu, exec_args= true,
	 value= function(pn)
		 return newfield_prefs_config:get_data(pn).speed_mod
	 end},
	{name= "speed_type", menu= nesty_option_menus.enum_option,
	 translatable= true, value= function(pn)
		 return newfield_prefs_config:get_data(pn).speed_type
	 end,
	 args= {
		 name= "speed_type", enum= newfield_speed_types, fake_enum= true,
		 obj_get= function(pn) return newfield_prefs_config:get_data(pn) end,
		 get= function(pn, obj) return obj.speed_type end,
		 set= function(pn, obj, value)
			 if obj.speed_type == "multiple" and value ~= "multiple" then
				 obj.speed_mod= math.round(obj.speed_mod * 100)
			 elseif obj.speed_type ~= "multiple" and value == "multiple" then
				 obj.speed_mod= obj.speed_mod / 100
			 end
			 obj.speed_type= value
			 newfield_prefs_config:set_dirty(pn)
			 MESSAGEMAN:Broadcast("ConfigValueChanged", {
				config_name= newfield_prefs_config.name, field_name= "speed_type", value= value, pn= pn})
		 end,
	}},
	nesty_options.float_song_mod_val("MusicRate", -2, -1, -1, .5, 2, 1),
	nesty_options.float_song_mod_toggle_val("Haste", 1, 0),
	{name= "perspective", translatable= true, menu= nesty_option_menus.menu, args= perspective_mods},
	nesty_options.float_config_toggle_val(newfield_prefs_config, "reverse", -1, 1),
	nesty_options.float_config_val(newfield_prefs_config, "zoom", -2, -1, 0),
	{name= "chart_mods", translatable= true, menu= nesty_option_menus.menu, args= chart_mods},
	{name= "newskin", translatable= true, menu= nesty_option_menus.newskins},
	{name= "newskin_params", translatable= true, menu= nesty_option_menus.menu,
	 args= gen_noteskin_param_menu, req_func= show_noteskin_param_menu},
	{name= "shown_noteskins", translatable= true, menu= nesty_option_menus.shown_noteskins, args= {}},
	nesty_options.bool_config_val(newfield_prefs_config, "hidden"),
	nesty_options.bool_config_val(newfield_prefs_config, "sudden"),
	{name= "advanced_notefield_config", translatable= true, menu= nesty_option_menus.menu, args= notefield_config},
	{name= "gameplay_options", translatable= true, menu= nesty_option_menus.menu, args= gameplay_options},
	{name= "life_options", translatable= true, menu= nesty_option_menus.menu,
	 args= life_options},
	nesty_options.bool_song_mod_val("AssistClap"),
	nesty_options.bool_song_mod_val("AssistMetronome"),
	nesty_options.bool_song_mod_val("StaticBackground"),
	nesty_options.bool_song_mod_val("RandomBGOnly"),
	nesty_options.float_config_val(player_config, "ScreenFilter", -2, -1, 0),
	{name= "reload_newskins", translatable= true, type= "action",
	 execute= function() NEWSKIN:reload_skins() end},
}

local player_ready= {}

local function exit_if_both_ready()
	for i, pn in ipairs(GAMESTATE:GetHumanPlayers()) do
		if not player_ready[pn] then return end
	end
	SCREENMAN:GetTopScreen():StartTransitioningScreen("SM_GoToNextScreen")
end

local prev_explanation= {}
local function update_explanation(pn)
	local cursor_item= menus[pn]:get_cursor_item()
	if cursor_item then
		local new_expl= cursor_item.name or cursor_item.text
		local expl_com= "change_explanation"
		if cursor_item.explanation then
			new_expl= cursor_item.explanation
			expl_com= "translated_explanation"
		end
		if new_expl ~= prev_explanation[pn] then
			prev_explanation[pn]= new_expl
			explanations[pn]:playcommand(expl_com, {text= new_expl})
		end
	end
end

local function input(event)
	local pn= event.PlayerNumber
	if not pn then return end
	if not menus[pn] then return end
	if menu_stack_generic_input(menus, event)
	and event.type == "InputEventType_FirstPress" then
		if event.GameButton == "Back" then
			SCREENMAN:GetTopScreen():StartTransitioningScreen("SM_GoToPrevScreen")
		else
			player_ready[pn]= true
			ready_indicators[pn]:playcommand("show_ready")
			exit_if_both_ready()
		end
	else
		if player_ready[pn] and not menus[pn]:can_exit_screen() then
			player_ready[pn]= false
			ready_indicators[pn]:playcommand("hide_ready")
		end
	end
	update_explanation(pn)
end

local frame= Def.ActorFrame{
	OnCommand= function(self)
		SCREENMAN:GetTopScreen():AddInputCallback(input)
		for pn, menu in pairs(menus) do
			menu:push_menu_stack(nesty_option_menus.menu, base_options, "play_song")
			menu:update_cursor_pos()
			update_explanation(pn)
		end
	end,
}
local item_params= {
	text_commands= {
		Font= "Common Normal", OnCommand= function(self)
			self:rotationz(720):linear(1):rotationz(0)
		end,
	},
	text_width= .7,
	value_text_commands= {
		Font= "Common Normal", OnCommand= function(self)
			self:rotationz(-720):linear(1):rotationz(0)
		end,
	},
	value_image_commands= {
		OnCommand= function(self)
			self:rotationz(-720):linear(1):rotationz(0)
		end,
	},
	value_width= .25,
	type_images= {
		bool= THEME:GetPathG("", "menu_icons/bool"),
		choice= THEME:GetPathG("", "menu_icons/bool"),
		menu= THEME:GetPathG("", "menu_icons/menu"),
	},
}
for pn, menu in pairs(menus) do
	frame[#frame+1]= LoadActor(
		THEME:GetPathG("ScreenOptions", "halfpage")) .. {
		InitCommand= function(self)
			self:xy(menu_x[pn], 250)
		end
	}
	frame[#frame+1]= menu:create_actors{
		x= menu_x[pn], y= 96, width= menu_width, height= menu_height,
		translation_section= "newfield_options",
		num_displays= 1, pn= pn, el_height= 20,
		menu_sounds= {
			pop= THEME:GetPathS("Common", "Cancel"),
			push= THEME:GetPathS("_common", "row"),
			act= THEME:GetPathS("Common", "value"),
			move= THEME:GetPathS("_switch", "down"),
			move_up= THEME:GetPathS("_switch", "up"),
			move_down= THEME:GetPathS("_switch", "down"),
			inc= THEME:GetPathS("_switch", "up"),
			dec= THEME:GetPathS("_switch", "down"),
		},
		display_params= {
			el_zoom= .55, item_params= item_params, item_mt= nesty_items.value,
			on= function(self)
				self:rotationx(720):linear(1):rotationx(0)
			end},
	}
	frame[#frame+1]= Def.BitmapText{
		Font= "Common Normal", InitCommand= function(self)
			explanations[pn]= self
			self:xy(menu_x[pn] - (menu_width / 2), _screen.cy+174)
				:diffuse(PlayerColor(pn))
				:shadowlength(1):wrapwidthpixels(menu_width / .5):zoom(.5)
				:horizalign(left)
		end,
		change_explanationCommand= function(self, param)
			local text= ""
			if THEME:HasString("newfield_explanations", param.text) then
				text= THEME:GetString("newfield_explanations", param.text)
			end
			self:playcommand("translated_explanation", {text= text})
		end,
		translated_explanationCommand= function(self, param)
			self:stoptweening():settext(param.text):cropright(1):linear(.5):cropright(0)
		end,
	}
	frame[#frame+1]= Def.BitmapText{
		Font= "Common Normal", Text= "READY!", InitCommand= function(self)
			ready_indicators[pn]= self
			self:xy(menu_x[pn], 106):zoom(1.5):diffuse(Color.Green):diffusealpha(0)
		end,
		show_readyCommand= function(self)
			self:stoptweening():decelerate(.5):diffusealpha(1)
		end,
		hide_readyCommand= function(self)
			self:stoptweening():accelerate(.5):diffusealpha(0)
		end,
	}
	local metrics_name = "PlayerNameplate" .. ToEnumShortString(pn)
	frame[#frame+1] = LoadActor(
		THEME:GetPathG("ScreenPlayerOptions", "PlayerNameplate"), pn) .. {
		InitCommand=function(self)
			self:name(metrics_name)
			ActorUtil.LoadAllCommandsAndSetXY(self,"ScreenPlayerOptions")
			self:x(menu_x[pn])
		end
	}
end

return frame
