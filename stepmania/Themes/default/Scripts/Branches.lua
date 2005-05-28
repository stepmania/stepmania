function ScreenTitleBranch()
	if GAMESTATE:GetCoinMode() == COIN_MODE_HOME then return "ScreenTitleMenu" end
	return "ScreenTitleJoin"
end

function ScreenCautionBranch()
	if PREFSMAN:GetPreference("ShowCaution") then return "ScreenCaution" end
	return "ScreenSelectStyle"
end

function SongSelectionScreen()
	if PlayModeName() == "Nonstop" then return "ScreenSelectCourseNonstop" end
	if PlayModeName() == "Oni" then return "ScreenSelectCourseOni" end
	if PlayModeName() == "Endless" then return "ScreenSelectCourseEndless" end
	if IsNetConnected() then ReportStyle() end
	if IsNetSMOnline() then return SMOnlineScreen() end
	if IsNetConnected() then return "ScreenNetSelectMusic" end
	return "ScreenSelectMusic"
end

function SMOnlineScreen()
	if ( not IsSMOnlineLoggedIn(1) ) and IsPlayerEnabled(1) then return "ScreenSMOnlineLogin" end
	if ( not IsSMOnlineLoggedIn(2) ) and IsPlayerEnabled(2) then return "ScreenSMOnlineLogin" end
	return "ScreenNetRoom"
end	

function SelectFirstOptionsScreen()
	if PlayModeName() == "Rave" then return "ScreenRaveOptions" end
	return "ScreenPlayerOptions"
end

function GetGameplayScreen()
	if IsExtraStage() or IsExtraStage2() then return "ScreenGameplay" end
	return "ScreenGameplay"
end

function SelectEvaluationScreen()
	if IsNetConnected() then return "ScreenNetEvaluation" end
	Mode = PlayModeName()
	if( Mode == "Regular" ) then return "ScreenEvaluationStage" end
	if( Mode == "Nonstop" ) then return "ScreenEvaluationNonstop" end
	if( Mode == "Oni" ) then return "ScreenEvaluationOni" end
	if( Mode == "Endless" ) then return "ScreenEvaluationEndless" end
	if( Mode == "Rave" ) then return "ScreenEvaluationRave" end
	if( Mode == "Battle" ) then return "ScreenEvaluationBattle" end
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
	if GetBestFinalGrade() >= Grade("AA") then return "ScreenMusicScroll" end
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

