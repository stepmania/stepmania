local menu_height= 410
local menu_width= 346
local menu_x= _screen.w * .30

local options= {
	{"item", theme_config, "AutoSetStyle", "bool"},
	{"item", theme_config, "LongFail", "bool"},
	{"item", theme_config, "ComboOnRolls", "bool"},
	{"item", theme_config, "FancyUIBG", "bool"},
	{"item", theme_config, "TimingDisplay", "bool"},
	{"item", theme_config, "GameplayFooter", "bool"},
	{"item", theme_config, "Use12HourClock", "bool"},
	{"item", theme_config, "menu_mode", "choice", {choices= {"two_direction", "two_direction_with_select", "four_direction"}}},
}

local menu_controller= setmetatable({}, menu_controller_mt)
local menu_sounds= {}

local prev_mx= INPUTFILTER:GetMouseX()
local prev_my= INPUTFILTER:GetMouseY()
local function update(delta)
	local mx= INPUTFILTER:GetMouseX()
	local my= INPUTFILTER:GetMouseY()
	if mx ~= prev_mx or my ~= prev_my then
		local sound_name= menu_controller:update_focus(mx, my)
		nesty_menus.play_menu_sound(menu_sounds, sound_name)
		prev_mx= mx
		prev_my= my
	end
end

local function input(event)
	local levels_left, sound_name= menu_controller:input(event)
	nesty_menus.play_menu_sound(menu_sounds, sound_name)
	if levels_left and levels_left < 1 then
		local metrics_need_to_be_reloaded= theme_config:check_dirty()
		theme_config:save()
		if metrics_need_to_be_reloaded then
			THEME:ReloadMetrics()
		end
		SCREENMAN:GetTopScreen():StartTransitioningScreen("SM_GoToNextScreen")
	end
end

return Def.ActorFrame{
	OnCommand= function(self)
		menu_sounds= nesty_menus.make_menu_sound_lookup(self)
		menu_controller:init{
			actor= self:GetChild("menu"), input_mode= theme_config:get_data().menu_mode,
			repeats_to_big= 10, select_goes_to_top= true, data= options,
			translation_section= "OptionTitles",
		}
		menu_controller:open_menu()
		SCREENMAN:GetTopScreen():AddInputCallback(input)
		self:SetUpdateFunction(update)
	end,
	MenuValueChangedMessageCommand= function(self, params)
		nesty_menus.handle_menu_refresh_message(params, {menu_controller})
	end,
	LoadActor(THEME:GetPathG("", "generic_menu.lua"), 1, 880, menu_height, 1, menu_x-(menu_width/2), 138, 36),
}
