local options= {
	nesty_options.bool_config_val(theme_config, "AutoSetStyle"),
	nesty_options.bool_config_val(theme_config, "LongFail"),
	nesty_options.bool_config_val(theme_config, "ComboOnRolls"),
	nesty_options.bool_config_val(theme_config, "FancyUIBG"),
	nesty_options.bool_config_val(theme_config, "TimingDisplay"),
	nesty_options.bool_config_val(theme_config, "GameplayFooter"),
	nesty_options.bool_config_val(theme_config, "Use12HourClock"),
}

local menu= setmetatable({}, nesty_menu_stack_mt)

local function input(event)
	if event.type == "InputEventType_Release" then return end
	local button= event.GameButton
	if not button then return end
	local menu_action= menu:interpret_code(button)
	if menu_action == "close" then
		local metrics_need_to_be_reloaded= theme_config:check_dirty()
		theme_config:save()
		if metrics_need_to_be_reloaded then
			THEME:ReloadMetrics()
		end
		SCREENMAN:GetTopScreen():StartTransitioningScreen("SM_GoToNextScreen")
	end
end

local item_params= {
	text_commands= {
		Font= "Common Condensed", OnCommand= function(self)
			self:diffuse(color("#512232")):diffusealpha(0):linear(1):diffusealpha(1)
		end,
	},
	text_width= .7,
	value_text_commands= {
		Font= "Common Normal", OnCommand= function(self)
			self:diffusealpha(0):linear(1):diffusealpha(1)
		end,
	},
	value_image_commands= {
		OnCommand= function(self)
			self:diffusealpha(0):linear(1):diffusealpha(1)
		end,
	},
	value_width= .25,
	type_images= {
		bool= THEME:GetPathG("", "menu_icons/bool"),
		choice= THEME:GetPathG("", "menu_icons/bool"),
		menu= THEME:GetPathG("", "menu_icons/menu"),
	},
}

return Def.ActorFrame{
	OnCommand= function(self)
		menu:push_menu_stack(nesty_option_menus.menu, options, "Exit Menu")
		menu:update_cursor_pos()
		SCREENMAN:GetTopScreen():AddInputCallback(input)
	end,
	menu:create_actors{
		x= _screen.cx, y= _screen.cy-280, width= 760,
		height= _screen.h*.75, num_displays= 1, pn= nil, el_height= 32,
		menu_sounds= {
			pop= THEME:GetPathS("Common", "Cancel"),
			push= THEME:GetPathS("_common", "row"),
			act= THEME:GetPathS("Common", "value"),
			move= THEME:GetPathS("Common", "value"),
			move_up= THEME:GetPathS("Common", "value"),
			move_down= THEME:GetPathS("Common", "value"),
			inc= THEME:GetPathS("_switch", "up"),
			dec= THEME:GetPathS("_switch", "down"),
		},
		display_params= {
			el_zoom= 1, item_params= item_params, item_mt= nesty_items.value,
			on= function(self)
				self:diffusealpha(0):decelerate(0.5):diffusealpha(1)
			end},
	},
}