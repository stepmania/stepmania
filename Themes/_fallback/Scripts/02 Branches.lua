--[[
[en] The Branch table replaces the various functions used for branching in the
StepMania 4 default theme.
Lines with a single string (e.g. TitleMenu = "ScreenTitleMenu") are referenced
in the metrics as Branch.keyname.
If the line is a function, you'll have to use Branch.keyname() instead.
--]]

-- used for various SMOnline-enabled screens:
function SMOnlineScreen()
	for pn in ivalues(GAMESTATE:GetHumanPlayers()) do
		if not IsSMOnlineLoggedIn(pn) then return "ScreenSMOnlineLogin" end;
	end;
	return "ScreenNetRoom"
end;

function SelectMusicOrCourse()
	if IsNetSMOnline() then
		return "ScreenNetSelectMusic";
	elseif GAMESTATE:IsCourseMode() then
		return "ScreenSelectCourse";
	else
		return "ScreenSelectMusic";
	end;
end;

Branch = {
	TitleMenu = "ScreenTitleMenu",
	AfterProfileLoad = function()
		if ( THEME:GetMetric("Common","AutoSetStyle") == true ) then
			if IsNetConnected() then
				-- use SelectPlayMode in online...
				return "ScreenSelectPlayMode"
			else
				return "ScreenSelectPlayStyle"
			end
		else
			return "ScreenSelectPlayMode"
		end
	end,
	AfterSelectPlayMode = function()
		if IsNetConnected() then
			ReportStyle()
			GAMESTATE:ApplyGameCommand("playmode,regular")
		end
		if IsNetSMOnline() then return SMOnlineScreen() end
		if IsNetConnected() then return "ScreenNetRoom" end
		return "ScreenSelectPlayStyle"
	end,
	AfterSelectStyle = function()
		if CHARMAN:GetAllCharacters() ~= nil then
			return "ScreenSelectCharacter"
		else
			return "ScreenGameInformation"
		end
	end,
	AfterProfileSave = function()
		-- Might be a little too broken? -- Midiman
		if GAMESTATE:IsEventMode() then
			return SelectMusicOrCourse()
		elseif STATSMAN:GetCurStageStats():AllFailed() then
			return "ScreenGameOver"
		elseif GAMESTATE:GetSmallestNumStagesLeftForAnyHumanPlayer() == 0 then
			return "ScreenEvaluationSummary"
		else
			return SelectMusicOrCourse()
		end
	end,
	GetGameInformationScreen = function()
		bTrue = PREFSMAN:GetPreference("ShowInstructions");
		return (bTrue and GoToMusic() or "ScreenGameInformation");
	end,
	AfterSMOLogin = SMOnlineScreen(),
	BackOutOfPlayerOptions = function()
		return SelectMusicOrCourse()
	end,
	BackOutOfStageInformation = function()
		return SelectMusicOrCourse()
	end,
	AfterSelectMusic = function()
		if SCREENMAN:GetTopScreen():GetGoToOptions() then
			return SelectFirstOptionsScreen()
		else
			return "ScreenStageInformation"
		end
	end,
	PlayerOptions = function()
		if SCREENMAN:GetTopScreen():GetGoToOptions() then
			return "ScreenPlayerOptions";
		else
			return "ScreenStageInformation";
		end
	end,
	SongOptions = function()
		if SCREENMAN:GetTopScreen():GetGoToOptions() then
			return "ScreenSongOptions"
		else
			return "ScreenStageInformation"
		end
	end,
	AfterGameplay = function()
		-- pick an evaluation screen based on settings.
		if IsNetSMOnline() then
			return "ScreenNetEvaluation"
		else
			-- todo: account for courses etc?
			return "ScreenEvaluationNormal"
		end
	end,
	AfterEvaluation = function()
		if GAMESTATE:GetSmallestNumStagesLeftForAnyHumanPlayer() >= 1 then
			return "ScreenProfileSave"
		else
			return "ScreenEvaluationSummary"
		end
	end,
	AfterSummary = "ScreenProfileSaveSummary",
	Network = function()
		return IsNetConnected() and "ScreenTitleMenu" or "ScreenTitleMenu"
	end,
	QuickSetupStart = "ScreenQuickSetupOverview",
	QuickSetupA = "ScreenQuickSetupPhaseOne",
	QuickSetupB = "ScreenQuickSetupPhaseTwo",
	QuickSetupC = "ScreenQuickSetupPhaseThree",
	QuickSetupD = "ScreenQuickSetupPhaseFour",
	QuickSetupFinished = "ScreenQuickSetupFinished",
};