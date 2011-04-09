--SplitMode (no version)
--[[--------------------------------------------------------------------------
Inspired by FSX's SplitLoader (ExSongs)
A Note: No code was stolen from FSX's script, this was completely made from
scratch (the only stolen thing was the idea of using the description as helper
lol) This is a minimized version, without accurate searching modes, prefs and
stuff. This only searches for a song specified in the description and sets
it along with the same kind of source difficulty and stepstype.

~{ How to use? }~
Write into steps description:
!SONG=<songname>
This script will do the rest...

~{ Where to use? }~
Anywhere before gameplay starts
(Recommended: ScreenStage or ScreenStageInformation)

~{ TODO list }~
>none...

~{ Credits }~
FSX: he had the initial ex-songs idea, I arranged that idea to make it
simplier and somewhat flexible

~{ License }~
cc-by
http://creativecommons.org/licenses/by/3.0/
In a nutshell: You are free to use this freely without worry, just atribute me
--]]--------------------------------------------------------------------------
function SplitMode()
	local bImWorking = false
	
	--local function Tracef(...) Trace(string.format(...)) end
	
	--I'll work with the master player
	local masterplayer = GAMESTATE:GetMasterPlayerNumber()
	local song = GAMESTATE:GetCurrentSong()
	local step = GAMESTATE:GetCurrentSteps(masterplayer)
	local description = step:GetDescription()
	--Working for 1 player atm...
	
	--if string.lower( step:GetDifficulty() ) ~= 'difficulty_edit' then
	if not step:IsAPlayerEdit() then
		if description ~= "" then
			--the description it's multipurpose
			local parts = split("|", description);
			local working = {}
			
			for i=1,#parts do
				if string.find(parts[i], "!SONG") then
					--use equals as separator
					working = split("=", parts[i])
				end
			end
			
			if #working == 2 then
				--two parts
				if working[1] == "!SONG" and working[2] ~= "" then
					local songtoset = SONGMAN:FindSong(working[2])
					--Lets check if there is a song...
					if songtoset ~= nil then
						local steptoset = songtoset:GetOneSteps(step:GetStepsType(), step:GetDifficulty())
						if steptoset ~= nil then
							--Yay!
							GAMESTATE:SetCurrentSong(songtoset)
							--Set MasterPlayer's steps to both players.
							for idx, player in ipairs(PlayerNumber) do
								GAMESTATE:SetCurrentSteps(player, steptoset)
							end
							bImWorking = true
							Trace("[SPLITMODE]: Split System Is Working...")
							Tracef("--> NewSong: '%s - %s'", songtoset:GetDisplayFullTitle(), songtoset:GetDisplayArtist())
							--Tracef("--> NewDifficulty: '%s'",steptoset:GetStepsType())
							--Tracef("--> NewStepType: '%s'",steptoset:GetDifficulty())
							Trace("--> Difficulty and StepsType should be the same as the source song")
						else
							Tracef("[SPLITMODE]: Steps with StepsType '%s' and Difficulty '%s' not found for the song '%s'...", step:GetStepsType(), step:GetDifficulty(), parts[2])
							--Tracef("[SPLITMODE]: No StepsType and Difficulty Match for song '%s'...", parts[2]);
						end
					else
						Tracef("[SPLITMODE]: Song '%s' not found...", working[2]);
					end
				else
					Trace("[SPLITMODE]: Do nothing: Malformed Working Part Format")
				end
			else
				Trace("[SPLITMODE]: Do nothing: Regular or Reserved Description")
			end
		else
			Trace("[SPLITMODE]: Do nothing: Empty Description")
		end
	else
		Trace("[SPLITMODE]: Do nothing: Secure Exception -> This is either an edit or a player edit")
	end
	
--	if not bImWorking then
--		Trace("[SPLITMODE]: Not working for an unknown reason...")
--	end
	
	return bImWorking
end