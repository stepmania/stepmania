function SongSelectionScreen()
	if PlayModeName() == "Nonstop" then return "ScreenSelectCourseNonstop" end
	if PlayModeName() == "Oni" then return "ScreenSelectCourseOni" end
	if PlayModeName() == "Endless" then return "ScreenSelectCourseEndless" end
	if IsNetConnected() then ReportStyle() end
	if IsNetSMOnline() then return "ScreenSMOnlineLogin" end
	if IsNetConnected() then return "ScreenNetSelectMusic" end
	return "ScreenSelectMusic"
end

function SelectGameplayScreen()
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

function SelectFirstOptionsScreen()
	if PlayModeName() == "Rave" then return ScreenRaveOptions end
	return "ScreenPlayerOptions"
end

function SelectEndingScreen()
	if GetBestFinalGrade() >= Grade("AAA") then return "ScreenMusicScroll" end
	return "ScreenCredits"
end	

