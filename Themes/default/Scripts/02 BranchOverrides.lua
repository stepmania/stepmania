Branch.OptionsEdit = function()
	if SONGMAN:GetNumSongs() == 0 and SONGMAN:GetNumAdditionalSongs() == 0 then
		return "ScreenHowToInstallSongs"
	end
	return "ScreenEditMenu"
end,
AfterContinue = function()
	if GAMESTATE:GetNumPlayersEnabled() == 0 then
		return "ScreenGameOver"
	end

	if STATSMAN:GetStagesPlayed() == 0 then
		if THEME:GetMetric("Common","AutoSetStyle") == false then
			return "ScreenSelectStyle"
		else
			return "ScreenProfileLoad"
		end
	end

	return "ScreenProfileLoad"
end