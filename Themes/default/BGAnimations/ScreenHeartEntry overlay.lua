local heart_xs= {
	[PLAYER_1]= SCREEN_WIDTH * .25,
	[PLAYER_2]= SCREEN_WIDTH * .75,
}

local heart_entries= {}
for i, pn in ipairs(GAMESTATE:GetEnabledPlayers()) do
	local profile= PROFILEMAN:GetProfile(pn)
	if profile and profile:GetIgnoreStepCountCalories() then
		heart_entries[pn]= new_numpad_entry{
			Name= pn .. "_heart_entry",
			InitCommand= cmd(xy, heart_xs[pn], SCREEN_CENTER_Y),
			value_color= PlayerColor(pn),
			cursor_draw= "first",
			cursor_color= PlayerDarkColor(pn),
			prompt_text= THEME:GetString("ScreenHeartEntry", "Heart Rate"),
			max_value= 300,
			auto_done_value= 100,
		}
	end
end

local function input(event)
	local pn= event.PlayerNumber
	if not pn then return end
	if event.type == "InputEventType_Release" then return end
	if not heart_entries[pn] then return end
	local done= heart_entries[pn]:handle_input(event.button)
	if done then
		SOUND:PlayOnce(THEME:GetPathS("Common", "Start"))
		local all_done= true
		for pn, entry in pairs(heart_entries) do
			if not entry.done then all_done= false break end
		end
		if all_done then
			for pn, entry in pairs(heart_entries) do
				local profile= PROFILEMAN:GetProfile(pn)
				if profile and profile:GetIgnoreStepCountCalories() then
					local calories= profile:CalculateCaloriesFromHeartRate(
						entry.value, GAMESTATE:GetLastGameplayDuration())
					profile:AddCaloriesToDailyTotal(calories)
				end
			end
			SCREENMAN:GetTopScreen():StartTransitioningScreen("SM_GoToNextScreen")
		end
	end
end

local timer_text
local function timer_update(self)
	local time= math.floor((self:GetSecsIntoEffect() % 60) * 10) / 10
	if time < 10 then
		timer_text:settext(("0%.1f"):format(time))
	else
		timer_text:settext(("%.1f"):format(time))
	end
end

local args= {
	Def.ActorFrame{
		Name= "timer",
		InitCommand= function(self)
			self:effectperiod(2^16)
			timer_text= self:GetChild("timer_text")
			self:SetUpdateFunction(timer_update)
		end,
		OnCommand= function(self)
			SCREENMAN:GetTopScreen():AddInputCallback(input)
		end,
		Def.BitmapText{
			Name= "timer_text", Font= "Common Normal", Text= "00.0",
			InitCommand= cmd(xy, SCREEN_CENTER_X, SCREEN_CENTER_Y; diffuse, Color.White)
		}
	},
	Def.BitmapText{
		Name= "explanation", Font= "Common Normal",
		Text= THEME:GetString("ScreenHeartEntry", "Enter Heart Rate"),
		InitCommand= cmd(xy, SCREEN_CENTER_X, SCREEN_CENTER_Y-120; diffuse, Color.White)},
	Def.BitmapText{
		Name= "song_len_label", Font= "Common Normal",
		Text= THEME:GetString("ScreenHeartEntry", "Song Length"),
		InitCommand= cmd(xy, SCREEN_CENTER_X, SCREEN_CENTER_Y-72; diffuse, Color.White)},
	Def.BitmapText{
		Name= "song_len", Font= "Common Normal",
		Text= SecondsToMMSS(GAMESTATE:GetLastGameplayDuration()),
		InitCommand= cmd(xy, SCREEN_CENTER_X, SCREEN_CENTER_Y-48; diffuse, Color.White)},
}

for pn, entry in pairs(heart_entries) do
	args[#args+1]= entry:create_actors()
end

return Def.ActorFrame(args)
