function ScreenTitleBranch()
	-- Don't show the title menu (says "Press Start") if there are 0 credits
	-- and inserted and CoinMode is pay.
	if GAMESTATE:GetCoinsNeededToJoin() > GAMESTATE:GetCoins() then
		return THEME:GetMetric( "Common", "InitialScreen" )
	end

	if GAMESTATE:GetCoinMode() == COIN_MODE_HOME then return "ScreenTitleMenu" end
	return "ScreenTitleJoin"
end

function ScreenCautionBranch()
	if PREFSMAN:GetPreference("ShowCaution") then return "ScreenCaution" end
	return "ScreenSelectStyle"
end

function SongSelectionScreen()
	local pm = GAMESTATE:GetPlayMode()
	if pm==PLAY_MODE_NONSTOP	then return "ScreenSelectCourseNonstop" end
	if pm==PLAY_MODE_ONI		then return "ScreenSelectCourseOni" end
	if pm==PLAY_MODE_ENDLESS	then return "ScreenSelectCourseEndless" end
	if IsNetConnected() then ReportStyle() end
	if IsNetSMOnline() then return SMOnlineScreen() end
	if IsNetConnected() then return "ScreenNetSelectMusic" end
	return "ScreenSelectMusic"
end

function SMOnlineScreen()
	if not IsSMOnlineLoggedIn(PLAYER_1) and GAMESTATE:IsPlayerEnabled(PLAYER_1) then return "ScreenSMOnlineLogin" end
	if not IsSMOnlineLoggedIn(PLAYER_2) and GAMESTATE:IsPlayerEnabled(PLAYER_2) then return "ScreenSMOnlineLogin" end
	return "ScreenNetRoom"
end	

function GetSelectMusicNext()
	if SCREENMAN:GetTopScreen():GetGoToOptions() then
		return SelectFirstOptionsScreen()
	else
		return "ScreenStage"
	end
end

function SelectFirstOptionsScreen()
	if GAMESTATE:GetPlayMode() == PLAY_MODE_RAVE then return "ScreenRaveOptions" end
	return "ScreenPlayerOptions"
end

function ScreenPlayerOptionsNext()
	if SCREENMAN:GetTopScreen():GetGoToOptions() then
		return "ScreenSongOptions"
	else
		return "ScreenStage"
	end
end

function GetGameplayScreen()
	if IsExtraStage() or IsExtraStage2() then return "ScreenGameplay" end
	return "ScreenGameplay"
end

function SelectEvaluationScreen()
	if IsNetConnected() then return "ScreenNetEvaluation" end
	local pm = GAMESTATE:GetPlayMode()
	if( GAMESTATE:GetMultiplayer() ) then return "ScreenEvaluationMultiplayer" end
	if( pm==PLAY_MODE_REGULAR )		then return "ScreenEvaluationStage" end
	if( pm==PLAY_MODE_NONSTOP )		then return "ScreenEvaluationNonstop" end
	if( pm==PLAY_MODE_ONI )			then return "ScreenEvaluationOni" end
	if( pm==PLAY_MODE_ENDLESS )		then return "ScreenEvaluationEndless" end
	if( pm==PLAY_MODE_RAVE )		then return "ScreenEvaluationRave" end
	if( pm==PLAY_MODE_BATTLE )		then return "ScreenEvaluationBattle" end
end

function GetTopScreenMetric( sElement )
	local sClass = SCREENMAN:GetTopScreen():GetName();
	return THEME:GetMetric( sClass, sElement )
end

function GetEvaluationNextScreen( sNextScreen, sFailedScreen, sEndScreen )
	Trace( "GetEvaluationNextScreen: " .. sNextScreen .. ", " .. sFailedScreen .. ", " .. sEndScreen )

	if GAMESTATE:IsEventMode() then
		Trace( "IsEventMode" )
		return sNextScreen
	end

	-- Not in event mode.  If failed, go to the game over screen.
	if STATSMAN:GetCurStageStats():AllFailed() then
		Trace( "Failed" )
		return sFailedScreen
	end

	local sIsStage = GetTopScreenMetric( "Summary" ) == "0" and not GAMESTATE:IsCourseMode()
	if sIsStage then
		local bHasAnotherStage = GAMESTATE:HasEarnedExtraStage()
		bHasAnotherStage = bHasAnotherStage or not
			(GAMESTATE:IsFinalStage() or GAMESTATE:IsExtraStage() or GAMESTATE:IsExtraStage2() )
		if bHasAnotherStage then
			Trace( "Another" )
			return sNextScreen
		end
	end

	Trace( "End" )
	return sEndScreen
end

function ScreenEvaluationExitBranch()
	if( IsNetSMOnline() ) then return "ScreenNetRoom" end
	if( IsNetConnected() ) then return "ScreenNetSelectMusic" end
	return "ScreenSelectMusic"
end

function ScreenBranchNetAfterEval()
	if IsNetSMOnline() then return "ScreenSMOnlineSelectMusic" end
	if IsNetConnected() then return "ScreenNetSelectMusic" end
	return "ScreenSelectMusic"
end	

function SelectEndingScreen()
	if GetBestFinalGrade() >= Grade_Tier03 then return "ScreenMusicScroll" end
	return "ScreenCredits"
end	

function IsEventMode()
	return PREFSMAN:GetPreference( "EventMode" )
end

-- For "EvalOnFail", do:
-- function GetGameplayNextScreen() return SelectEvaluationScreen() end

function GetGameplayNextScreen()
	Trace( "GetGameplayNextScreen: " )
	local Passed = not AllFailed()
	Trace( " Passed = "..tostring(Passed) )
	Trace( " IsSyncDataChanged = "..tostring(GAMESTATE:IsSyncDataChanged()) )
	Trace( " IsCourseMode = "..tostring(GAMESTATE:IsCourseMode()) )
	Trace( " IsExtraStage = "..tostring(IsExtraStage()) )
	Trace( " IsExtraStage2 = "..tostring(IsExtraStage2()) )
	Trace( " Event mode = "..tostring(IsEventMode()) )
	
	if GAMESTATE:IsSyncDataChanged() then 
		return "ScreenSaveSync"
	end

	if Passed or GAMESTATE:IsCourseMode() or
		IsExtraStage() or IsExtraStage2()
	then
		Trace( "Go to evaluation screen" )
		return SelectEvaluationScreen()
	end

	if IsEventMode() then
		Trace( "Go to song selection screen" )
		-- DeletePreparedScreens()
		return SongSelectionScreen()
	end

	Trace( "ScreenGameOver" )
	return "ScreenGameOver"
end

local function ShowScreenInstructions()
	if not PREFSMAN:GetPreference("ShowInstructions") then
		return false
	end

	if GAMESTATE:GetPlayMode() == PLAY_MODE_INVALID then
		Trace( "ShowScreenInstructions: called without PlayMode set" )
		return true
	end

	if GAMESTATE:GetPlayMode() ~= PLAY_MODE_REGULAR then
		return true
	end

	for pn = PLAYER_1,NUM_PLAYERS-1 do
		if GAMESTATE:GetPreferredDifficulty(pn) <= DIFFICULTY_EASY then
			return true
		end
	end

	return false
end

function GetScreenInstructions()
	if not ShowScreenInstructions() then
		return THEME:GetMetric("ScreenInstructions","NextScreen")
	else
		return "ScreenInstructions"
	end
end

function OptionsMenuAvailable()
	if GAMESTATE:IsExtraStage() or GAMESTATE:IsExtraStage2() then return false end
	return true
end

function ModeMenuAvailable()
	local PickExtraStage = PREFSMAN:GetPreference( "PickExtraStage" )
	if (GAMESTATE:IsExtraStage() and not PickExtraStage) or GAMESTATE:IsExtraStage2() then
		return false
	end
	return true
end

-- (c) 2005 Glenn Maynard, Chris Danford
-- All rights reserved.
-- 
-- Permission is hereby granted, free of charge, to any person obtaining a
-- copy of this software and associated documentation files (the
-- "Software"), to deal in the Software without restriction, including
-- without limitation the rights to use, copy, modify, merge, publish,
-- distribute, and/or sell copies of the Software, and to permit persons to
-- whom the Software is furnished to do so, provided that the above
-- copyright notice(s) and this permission notice appear in all copies of
-- the Software and that both the above copyright notice(s) and this
-- permission notice appear in supporting documentation.
-- 
-- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
-- OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
-- MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
-- THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
-- INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
-- OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
-- OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
-- OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
-- PERFORMANCE OF THIS SOFTWARE.

