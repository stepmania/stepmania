-- This example describes everything you need to do in your theme to allow the
-- player to enter their heart rate for calorie calculation.

-- First, the metrics you'll need to add to metrics.ini.
-- [ScreenHeartEntry]
-- # The HeartEntryEnabled metric must be set to true so the branching logic
-- # will know that your theme supports the screen.
-- HeartEntryEnabled=true
-- # ScreenHeartEntry uses ScreenWithMenuElements as its fallback, so the
-- # metrics in the ScreenWithMenuElements section will also apply to it.

-- Now for the body of the screen.  The things discussed here will have to go
-- in BGAnimations/ScreenHeartEntry overlay.lua

-- The positions the numpads for entering the heart rate will be placed at.
local heart_xs= {
	[PLAYER_1]= SCREEN_CENTER_X * 0.625,
	[PLAYER_2]= SCREEN_CENTER_X * 1.375,
}

-- heart_entries is a table that will be used to store the numpads that the
-- players will use.
local heart_entries= {}
-- Note that a numpad is only created for a player if that player is enabled
-- and they have the flag set in their profile to enable heart rate entry.
for i, pn in ipairs(GAMESTATE:GetEnabledPlayers()) do
	local profile= PROFILEMAN:GetProfile(pn)
	if profile and profile:GetIgnoreStepCountCalories() then
		-- This is probably the part you'll want to configure the most, the
		-- numpads.  _fallback/Scripts/04 NumPadEntry.lua discusses configuring a
		-- numpad in detail.
		heart_entries[pn]= new_numpad_entry{
			Name= pn .. "_heart_entry",
			InitCommand= cmd(xy, heart_xs[pn], SCREEN_CENTER_Y+48),
			-- Settings for value are optional, but you will probably want to
			-- change them, so they are provided in this example.
			-- If a simple colored BitmapText isn't what you want for displaying
			-- the value being entered, read _fallback/Scripts/04 NumPadEntry.lua.
			value_pos= {0, -48},
			value_font= "Common Normal",
			value_color= PlayerColor(pn),
			-- Optional prompt settings.
			prompt_pos= {0, -72},
			prompt_font= "Common Normal",
			prompt_color= Color.White,
			prompt_text= THEME:GetString("ScreenHeartEntry", "Heart Rate"),
			-- Optional button settings.
			button_positions= {
				{-30, -24}, {0, -24}, {30, -24},
				{-30, 0},   {0, 0},   {30, 0},
				{-30, 24}, {0, 24},   {30, 24},
				{-30, 48}, {0, 48},   {30, 48}
			},
			button_font= "Common Normal",
			button_color= Color.White,
			-- Cursor settings.  You probably want something more than a quad for
			-- the cursor, so this example includes the command the cursor needs
			-- to support to do its job.
			cursor= Def.Quad{
				InitCommand= function(self)
					self:SetWidth(16)
					self:SetHeight(28)
					self:diffuse(PlayerDarkColor(pn))
				end,
				-- MoveCommand is used to move the cursor to a button.
				MoveCommand= function(self, param)
					-- param contains the position to move to.
					self:stoptweening()
					self:decelerate(0.15)
					self:xy(param[1], param[2])
				end,
				-- FitCommand is used to change the size of the cursor to fit the
				-- button after it has been moved.
				FitCommand= function(self, param)
					-- param is the actor for the button.
					-- Note that this does not use stoptweening or finishtweening.
					-- This is because it is executed with playcommand immediately
					-- after MoveCommand, and thus the state changes it makes combine
					-- with the actor state at the end of MoveCommand.
					self:SetWidth(param:GetWidth())
				end
			},
			-- cursor_draw controls whether the cursor is above or below the
			-- buttons.
			cursor_draw= "first", -- stick it under the buttons.
			-- Don't let the player enter a silly value.
			max_value= 300,
			-- Automatically move the cursor to the done button when the player has
			-- entered three digits.
			auto_done_value= 100,
		}
	end
end

-- This function is the input callback.  It handles button presses from the
-- player, passing them to the numpad so the player can enter their heart
-- rate, and detecting when the player is done.
-- See the AddInputCallback function for the Screen class in Lua.xml for a
-- full description of the event argument.
local function input(event)
	local pn= event.PlayerNumber
	-- If the PlayerNumber isn't set, the button isn't mapped.  Ignore it.
	if not pn then return end
	-- If it's a release, ignore it.
	if event.type == "InputEventType_Release" then return end
	-- If the player doesn't have a heart_entry, ignore it.
	if not heart_entries[pn] then return end
	-- Pass the input to the heart_entry for the player.  It will handle moving
	-- the cursor.  If it returns true, the player is done.
	local done= heart_entries[pn]:handle_input(event.GameButton)
	if done then
		-- Play a sound for the player so they know the value was entered.
		SOUND:PlayOnce(THEME:GetPathS("Common", "Start"))
		-- Check whether all players that need to enter their heart rate are done.
		local all_done= true
		for pn, entry in pairs(heart_entries) do
			if not entry.done then all_done= false break end
		end
		if all_done then
			-- If the players are done, add the calories to their profiles.
			for pn, entry in pairs(heart_entries) do
				local profile= PROFILEMAN:GetProfile(pn)
				if profile and profile:GetIgnoreStepCountCalories() then
					-- calories is the value you might want to store off in a global
					-- variable to display on evaluation.
					local calories= profile:CalculateCaloriesFromHeartRate(
						entry.value, GAMESTATE:GetLastGameplayDuration())
					profile:AddCaloriesToDailyTotal(calories)
				end
			end
			-- Since all players are done, transition to the next screen.
			SCREENMAN:GetTopScreen():StartTransitioningScreen("SM_GoToNextScreen")
		end
	end
end

-- A simple timer that the player can watch while counting their pulse.
-- The menu timer isn't used because it has undesired side effects.
-- timer_text will be set to the BitmapText used to display the time.  This
-- way, the BitmapText doesn't have to be fetched with GetChild every frame.
local timer_text
-- This is an update function for an ActorFrame.  It will run every frame.
local function timer_update(self)
	local time= math.floor((self:GetSecsIntoEffect() % 60) * 10) / 10
	if time < 10 then
		timer_text:settext(("0%.1f"):format(time))
	else
		timer_text:settext(("%.1f"):format(time))
	end
end

-- args contains the actors that will be in the ActorFrame for this screen.
local args= {
	Def.ActorFrame{
		Name= "timer",
		InitCommand= function(self)
			-- Set the effectperiod so the timer can show an elapsed time.
			self:effectperiod(2^16)
			-- Set timer_text so GetChild doesn't need to be called every frame.
			timer_text= self:GetChild("timer_text")
			-- Set the update function so the timer will be updated.
			self:SetUpdateFunction(timer_update)
		end,
		OnCommand= function(self)
			-- Add the input callback so input will be handled.
			SCREENMAN:GetTopScreen():AddInputCallback(input)
		end,
		-- The BitmapText used to display the elapsed time.
		Def.BitmapText{
			Name= "timer_text", Font= "Common Normal", Text= "00.0",
			InitCommand= cmd(xy, SCREEN_CENTER_X, SCREEN_CENTER_Y-80; diffuse, Color.White),
			OnCommand= cmd(strokecolor,Color.Outline),
		}
	},
}

-- The actors for the heart_entries are added in a loop so that only the ones
-- that will be used will exist.
for pn, entry in pairs(heart_entries) do
	args[#args+1]= entry:create_actors()
end

return Def.ActorFrame(args)
