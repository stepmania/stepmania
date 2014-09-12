-- The positions of the numbers on the numpad.
local numpad_poses= {
	{-24, -24}, {0, -24}, {24, -24},
	{-24, 0},   {0, 0},   {24, 0},
	{-24, 24}, {0, 24},   {24, 24},
	{-24, 48}, {0, 48},   {24, 48}}
-- Some special escape sequences are converted to special characters.
local done_text= "&start;"
local back_text= "&leftarrow;"
local numpad_nums= {7, 8, 9, 4, 5, 6, 1, 2, 3, 0, done_text, back_text}
if MonthOfYear() == 3 and DayOfMonth() == 1 then
	for i= 1, #numpad_nums do
		local a= math.random(1, #numpad_nums)
		local b= math.random(1, #numpad_nums)
		numpad_nums[a], numpad_nums[b]= numpad_nums[b], numpad_nums[a]
	end
end

-- Each of these will be filled with an entry for each player that is
-- entering their heart rate.
-- This way, the actors are stored in convenient tables, so we don't have to go
-- through an inconvenient and inefficient chain of GetChild calls when we need
-- to change one of them.
local cursor_poses= {}
local cursor_quads= {}
local values= {}
local value_texts= {}
local done_status= {}

-- This creates the actor that will show the numpad, prompt, and value for a
-- player.  The PlayerNumber is passed in so that it can add its parts to the
-- tables above.
local function entry_pad(name, x, y, pn)
	local args= {
		Name= name, InitCommand= cmd(xy, x, y),
		-- The cursor actor.  First in the frame so it will be behind the text.
		Def.Quad{
			Name= "cursor", InitCommand= function(self)
				self:setsize(16, 24)
				self:diffuse(PlayerDarkColor(pn))
				cursor_quads[pn]= self
			end
		},
		Def.BitmapText{
			Name= "rate_label", Font= "Common Normal",
			Text= THEME:GetString("ScreenHeartEntry", "Heart Rate"),
			InitCommand= cmd(xy, 0, -72; diffuse, Color.White)
		},
		Def.BitmapText{
			Name= "value", Font= "Common Normal", Text= "0",
			InitCommand= function(self)
				self:xy(0, -48)
				self:diffuse(PlayerColor(pn))
				value_texts[pn]= self
			end
		}
	}
	-- A loop is used to add each of the numbers on the numpad to shorten the
	-- code and make it easier to add numbers later.
	for i, num in ipairs(numpad_nums) do
		args[#args+1]= Def.BitmapText{
			Name= "num"..i, Font= "Common Normal", Text= num,
			InitCommand= cmd(xy, numpad_poses[i][1], numpad_poses[i][2]; diffuse, Color.White)
		}
	end
	return Def.ActorFrame(args)
end

-- A function to update the cursor position for a player.
local function update_cursor(pn)
	local pos= numpad_poses[cursor_poses[pn]]
	cursor_quads[pn]:stoptweening()
	cursor_quads[pn]:linear(.1)
	cursor_quads[pn]:xy(pos[1], pos[2])
end

local function pad_input(pn, button)
	if button == "Start" then
		local num= numpad_nums[cursor_poses[pn]]
		local as_num= tonumber(num)
		if as_num then
			local new_value= (values[pn] * 10) + as_num
			if new_value < 300 then
				values[pn]= new_value
				if new_value > 100 then
					cursor_poses[pn]= 11
					update_cursor(pn)
				end
				value_texts[pn]:settext(values[pn])
			else
				SOUND:PlayOnce(THEME:GetPathS("Common", "Invalid"))
			end
			return false
		else
			if num == done_text then
				done_status[pn]= true
				return true
			elseif num == back_text then
				values[pn]= math.floor(values[pn]/10)
				value_texts[pn]:settext(values[pn])
				return false
			end
		end
	else
		local adds= {
			Left= {2, -1, -1},
			Right= {1, 1, -2},
			MenuLeft= {-1, -1, -1},
			MenuRight= {1, 1, 1},
			Up= {9, -3, -3, -3},
			Down= {3, 3, 3, -9}
		}
		adds.MenuUp= adds.Up
		adds.MenuDown= adds.Down
		local lr_buttons= {
			Left= true, Right= true, MenuLeft= true, MenuRight= true}
		if adds[button] then
			local index= 0
			if lr_buttons[button] then
				index= ((cursor_poses[pn] - 1) % 3) + 1
			else
				index= math.ceil(cursor_poses[pn] / 3)
			end
			local new_pos= cursor_poses[pn] + adds[button][index]
			if new_pos < 1 then new_pos= #numpad_nums end
			if new_pos > #numpad_nums then new_pos= 1 end
			cursor_poses[pn]= new_pos
			update_cursor(pn)
			return false
		end
	end
	return false
end

local function input(event)
	local pn= event.PlayerNumber
	Trace("input recieved")
	if not pn then return end
	Trace("has a pn")
	if event.type == "InputEventType_Release" then return end
	Trace("not a release")
	if not cursor_poses[pn] then return end
	Trace("has a cursor")
	local done= pad_input(pn, event.button)
	if done then
		local all_done= true
		for i, status in pairs(done_status) do
			if not status then all_done= false break end
		end
		if all_done then
			for pn, value in pairs(values) do
				local profile= PROFILEMAN:GetProfile(pn)
				if profile and profile:GetIgnoreStepCountCalories() then
					local calories= profile:CalculateCaloriesFromHeartRate(
						values[pn], GAMESTATE:GetLastGameplayDuration())
					profile:AddCaloriesToDailyTotal(calories)
				end
			end
			SOUND:PlayOnce(THEME:GetPathS("Common", "Start"))
			SCREENMAN:GetTopScreen():StartTransitioningScreen("SM_GoToNextScreen")
		end
	end
end

local heart_xs= {
	[PLAYER_1]= SCREEN_WIDTH * .25,
	[PLAYER_2]= SCREEN_WIDTH * .75,
}

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

for i, pn in ipairs(GAMESTATE:GetEnabledPlayers()) do
	local profile= PROFILEMAN:GetProfile(pn)
	if profile and profile:GetIgnoreStepCountCalories() then
		cursor_poses[pn]= 5
		values[pn]= 0
		done_status[pn]= false
		args[#args+1]= entry_pad(pn, heart_xs[pn], SCREEN_CENTER_Y, pn)
	end
end

return Def.ActorFrame(args)
