local menu_height= 400
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
	nesty_options.submenu("turn_chart_mods", turn_chart_mods),
	nesty_options.submenu("removal_chart_mods", removal_chart_mods),
	nesty_options.submenu("insertion_chart_mods", insertion_chart_mods),
}

local gameplay_options= {
	nesty_options.bool_config_val(player_config, "ComboUnderField"),
	nesty_options.bool_config_val(player_config, "FlashyCombo"),
	nesty_options.bool_config_val(player_config, "GameplayShowStepsDisplay"),
	nesty_options.bool_config_val(player_config, "GameplayShowScore"),
	nesty_options.bool_config_val(player_config, "JudgmentUnderField"),
	nesty_options.bool_config_val(player_config, "Protiming"),
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
	nesty_options.float_player_mod_val_new("BatteryLives", 1, 2, 1, 10, 4),
}

-- I suppose now would be a good time to mention that, since the float menu code ...thingy doesn't rely on powers of 10 anymore,
-- the values passed in will be different.
local base_options= {
	notefield_prefs_speed_type_menu(),
	notefield_prefs_speed_mod_menu(),
	--Turns out the music rate can't handle values higher than 3!
	nesty_options.float_song_mod_val_new("MusicRate", 0.1, 1, .1, 3, 1),
	nesty_options.float_song_mod_toggle_val("Haste", 1, 0),
	notefield_perspective_menu(),
	nesty_options.float_config_toggle_val(notefield_prefs_config, "reverse", -1, 1),
	nesty_options.float_config_val_new(notefield_prefs_config, "zoom", 0.1, 1),
	nesty_options.submenu("chart_mods", chart_mods),
	{name= "noteskin", translatable= true, menu= nesty_option_menus.noteskins},
	{name= "shown_noteskins", translatable= true, menu= nesty_option_menus.shown_noteskins, args= {}},
	nesty_options.bool_config_val(notefield_prefs_config, "hidden"),
	nesty_options.bool_config_val(notefield_prefs_config, "sudden"),
	advanced_notefield_prefs_menu(),
	nesty_options.submenu("gameplay_options", gameplay_options),
	nesty_options.submenu("life_options", life_options),
	nesty_options.bool_song_mod_val("AssistClap"),
	nesty_options.bool_song_mod_val("AssistMetronome"),
	nesty_options.bool_song_mod_val("StaticBackground"),
	nesty_options.bool_song_mod_val("RandomBGOnly"),
	nesty_options.float_config_val_new(player_config, "ScreenFilter", 0.1, 0.5, 0, 1),
	get_notefield_mods_toggle_menu(true, true),
	{name= "reload_noteskins", translatable= true, type= "action",
	 execute= function() NOTESKIN:reload_skins() end},
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
		Font= "Common Condensed", OnCommand= function(self)
			self:diffuse(color("#3D1D23")):diffusealpha(0):decelerate(0.2):diffusealpha(1)
		end,
		OffCommand=function(self)
			self:smooth(0.3):diffusealpha(0)
		end,
	},
	text_width= .7,
	value_text_commands= {
		Font= "Common Condensed", OnCommand= function(self)
			self:diffuse(color("#AC214A")):diffusealpha(0):decelerate(0.2):diffusealpha(1)
		end,
		OffCommand=function(self)
			self:smooth(0.3):diffusealpha(0)
		end,
	},
	value_image_commands= {
		OnCommand= function(self)
			self:diffusealpha(0):smooth(0.3):diffusealpha(1)
		end,
		OffCommand=function(self)
			self:smooth(0.3):diffusealpha(0)
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
			self:xy(menu_x[pn], 360)
		end;
		OnCommand=function(self)
			self:diffusealpha(0):zoomx(0.8):decelerate(0.3):diffusealpha(1):zoomx(1)
		end;
		OffCommand=function(self)
			self:decelerate(0.3):diffusealpha(0)
		end;
	}
	frame[#frame+1]= menu:create_actors{
		x= menu_x[pn], y= 120, width= menu_width, height= menu_height,
		translation_section= "notefield_options",
		num_displays= 1, pn= pn, el_height= 36,
		menu_sounds= {
			pop= THEME:GetPathS("Common", "Cancel"),
			push= THEME:GetPathS("_common", "row"),
			act= THEME:GetPathS("Common", "start"),
			move= THEME:GetPathS("Common", "value"),
			move_up= THEME:GetPathS("Common", "value"),
			move_down= THEME:GetPathS("Common", "value"),
			inc= THEME:GetPathS("_switch", "up"),
			dec= THEME:GetPathS("_switch", "down"),
		},
		display_params= {
			el_zoom= 0.8, item_params= item_params, item_mt= nesty_items.value, heading_height = 48,
			on= function(self)
				self:diffusealpha(0):decelerate(0.2):diffusealpha(1)
			end,
			off= function(self)
				self:decelerate(0.2):diffusealpha(0)
			end,	
			},
	}
	frame[#frame+1]= Def.BitmapText{
		Font= "Common Normal", InitCommand= function(self)
			explanations[pn]= self
			self:xy(menu_x[pn] - (menu_width / 2), _screen.cy+154)
				:diffuse(ColorDarkTone((PlayerColor(pn)))):horizalign(left):vertalign(top)		
				:wrapwidthpixels(menu_width / .8):zoom(.75)
				:horizalign(left)
		end,
		change_explanationCommand= function(self, param)
			local text= ""
			if THEME:HasString("notefield_explanations", param.text) then
				text= THEME:GetString("notefield_explanations", param.text)
			end
			self:playcommand("translated_explanation", {text= text})
		end,
		translated_explanationCommand= function(self, param)
			self:stoptweening():settext(param.text):cropright(1):linear(.5):cropright(0)
		end,
	}
	frame[#frame+1]= Def.BitmapText{
		Font= "Common Condensed", Text= "READY!", InitCommand= function(self)
			ready_indicators[pn]= self
			self:xy(menu_x[pn], 146):zoom(1.5):diffuse(Color.Green):strokecolor(color("#2E540F")):diffusealpha(0)
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
