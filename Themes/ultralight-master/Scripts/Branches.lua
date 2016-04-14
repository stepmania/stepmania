function SMOnlineScreen()
	for pn in ivalues(GAMESTATE:GetHumanPlayers()) do
		if not IsSMOnlineLoggedIn(pn) then
			return "ScreenSMOnlineLogin"
		end
	end
	return "ScreenNetRoom"
end

function SelectMusicOrCourse()
	if IsNetSMOnline() then 			 return "ScreenNetSelectMusic"
	elseif GAMESTATE:IsCourseMode() then return "ScreenSelectCourse"
	else								 return "ScreenSelectMusic"
	end
end

-- help
if not Branch then Branch = {} end

Branch.GameStart = function()
	local AutoSetStyle = THEME:GetMetric("Common","AutoSetStyle") == true
	return AutoSetStyle and "ScreenSelectNumPlayers" or "ScreenSelectStyle"
end

-- assumes a branch table exists in the fallback theme:
Branch.NetworkConnect = function()
	if IsNetSMOnline() then return "ScreenReportStyle" end
	return "ScreenServerConnect"
end

Branch.ReportStyle = function()
	if IsNetConnected() then ReportStyle() end
	if IsNetSMOnline() then return SMOnlineScreen() end
end

Branch.ServerConnectNext = function()
	if IsNetConnected() then return "ScreenReportStyle" end
	if IsNetSMOnline() then return SMOnlineScreen() end
	return "ScreenSelectMode" -- exiting while offline
end

Branch.PlayerOptions = function()
	local pm = GAMESTATE:GetPlayMode()
	local restricted = { "PlayMode_Oni", "PlayMode_Rave",
		--"PlayMode_Battle" -- has forced mods as gameplay...
	}
	for i=1,#restricted do
		if restricted[i] == pm then
			if SCREENMAN:GetTopScreen():GetGoToOptions() then
				return "ScreenPlayerOptionsRestricted"
			else
				return "ScreenStageInformation"
			end
		end
	end
	if SCREENMAN:GetTopScreen():GetGoToOptions() then
		return "ScreenPlayerOptions"
	else
		return "ScreenStageInformation"
	end
end

Branch.BackOutOfPlayerOptions = function()
	local env = GAMESTATE:Env()
	if env["Network"] then return "ScreenNetSelectMusic"
	else
		if GAMESTATE:IsCourseMode() then return "ScreenSelectCourse"
		else return "ScreenSelectMusic"
		end
	end
end

Branch.AfterContinue = function()
	-- Nobody joined? Show the game over.
	if GAMESTATE:GetNumPlayersEnabled() == 0 then return "ScreenGameOver" end

	-- The player is reset. This means a few things, but I need to do some testing.
	-- (might depend on value of AutoSetStyle.)
	local AutoSetStyle = THEME:GetMetric("Common","AutoSetStyle") == true

	if STATSMAN:GetStagesPlayed() == 0 then
		return AutoSetStyle and "ScreenProfileLoad" or "ScreenSelectStyle"
	end

	return "ScreenProfileLoad"
end
