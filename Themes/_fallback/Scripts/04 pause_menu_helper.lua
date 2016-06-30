local config_data= {
	pause_buttons= {Start= false, Select= true, Back= true},
	pause_tap_time= 1, pause_debounce_time= .1,
}
local pause_press_times= {}
local last_press_buttons= {}
local down_status= {}
local enabled_players= {}
local screen_gameplay= false
local pause_actor= false

local function other_button_down(pn, button)
	for other_pn, status in pairs(down_status) do
		for other_button, down in pairs(status) do
			if down and (other_button ~= button or other_pn ~= pn) then
				return true
			end
		end
	end
	return false
end

local function detect_lr_press(pn)
	return down_status[pn].MenuLeft and down_status[pn].MenuRight
end

local function input(event)
	local pn= event.PlayerNumber
	if not enabled_players[pn] then return end
	local button= event.button
	local game_button= event.GameButton
	if not game_button then return end
	if event.type == "InputEventType_Release" then
		down_status[pn][button]= false
		return
	end
	down_status[pn][button]= true
	if button ~= last_press_buttons[pn] then
		last_press_buttons[pn]= nil
	end
	if screen_gameplay:IsPaused() then return end
	if event.type ~= "InputEventType_FirstPress" then return end
	if detect_lr_press(pn) then
		MESSAGEMAN:Broadcast("PlayerHitPause", {pn= pn, button= button})
		return
	end
	if not config_data.pause_buttons[button] then
		MESSAGEMAN:Broadcast("HidePausePrompt")
		return
	end
	if last_press_buttons[pn] == button and pause_press_times[pn]
	and not other_button_down(pn, button) then
		local time_diff= GetTimeSinceStart() - pause_press_times[pn]
		if time_diff > config_data.pause_debounce_time then
			if time_diff < config_data.pause_tap_time then
				if GAMESTATE:GetCurMusicSeconds() > GAMESTATE:GetCurrentSong():GetFirstSecond() then
					gameplay_pause_count= gameplay_pause_count + 1
				end
				last_press_buttons[pn]= nil
				pause_press_times[pn]= nil
				MESSAGEMAN:Broadcast("PlayerHitPause", {pn= pn, button= button})
				return true
			else
				pause_press_times[pn]= GetTimeSinceStart()
				last_press_buttons[pn]= button
				MESSAGEMAN:Broadcast("ShowPausePrompt", {pn= pn, button= button})
			end
		end
	else
		pause_press_times[pn]= GetTimeSinceStart()
		last_press_buttons[pn]= button
		MESSAGEMAN:Broadcast("ShowPausePrompt", {pn= pn, button= button})
	end
end

function pause_controller_actor()
	gameplay_pause_count= 0
	return Def.Actor{
		OnCommand= function(self)
			screen_gameplay= SCREENMAN:GetTopScreen()
			if screen_gameplay:GetName() == "ScreenGameplaySyncMachine" then
				return
			end
			screen_gameplay:AddInputCallback(input)
			enabled_players= GAMESTATE:GetEnabledPlayers()
			for i, pn in ipairs(enabled_players) do
				enabled_players[pn]= true
				down_status[pn]= {}
			end
		end,
	}
end
