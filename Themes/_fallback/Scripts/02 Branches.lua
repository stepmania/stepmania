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
		if not IsSMOnlineLoggedIn(pn) then
			return "ScreenSMOnlineLogin"
		end
	end
	return "ScreenNetRoom"
end

function SelectMusicOrCourse()
	if IsNetSMOnline() then
		return "ScreenNetSelectMusic"
	elseif GAMESTATE:IsCourseMode() then
		return "ScreenSelectCourse"
	else
		return "ScreenSelectMusic"
	end
end

-- functions used for Routine mode
function IsRoutine()
	return GAMESTATE:GetCurrentStyle() and GAMESTATE:GetCurrentStyle():GetStyleType() == "StyleType_TwoPlayersSharedSides"
end

Branch = {
	Init = function()
		return "ScreenInit"
	end,
	AfterInit = function()
		if GAMESTATE:GetCoinMode() == 'CoinMode_Home' then
			return Branch.TitleMenu()
		else
			return "ScreenLogo"
		end
	end,
	TitleMenu = function()
		-- home mode is the most assumed use of sm-ssc.
		if GAMESTATE:GetCoinMode() == "CoinMode_Home" then
			return "ScreenTitleMenu"
		end
		-- arcade junk:
		if GAMESTATE:GetCoinsNeededToJoin() > GAMESTATE:GetCoins() then
			-- if no credits are inserted, don't show the Join screen. SM4 has
			-- this as the initial screen, but that means we'd be stuck in a
			-- loop with ScreenInit. No good.
			return "ScreenTitleJoin"
		else
			return "ScreenTitleJoin"
		end
	end,
	StartGame = function()
		-- Check to see if there are 0 songs installed. Also make sure to check
		-- that the additional song count is also 0, because there is
		-- a possibility someone will use their existing StepMania simfile
		-- collection with sm-ssc via AdditionalFolders/AdditionalSongFolders.
		if SONGMAN:GetNumSongs() == 0 and SONGMAN:GetNumAdditionalSongs() == 0 then
			return "ScreenHowToInstallSongs"
		end
		if PROFILEMAN:GetNumLocalProfiles() >= 2 then
			return "ScreenSelectProfile"
		else
			return "ScreenProfileLoad"
		end
	end,
	OptionsEdit = function()
		-- Similar to above, don't let anyone in here with 0 songs.
		if SONGMAN:GetNumSongs() == 0 and SONGMAN:GetNumAdditionalSongs() == 0 then
			return "ScreenHowToInstallSongs"
		end
		return "ScreenOptionsEdit"
	end,
	AfterProfileLoad = function()
		return Branch.AfterSelectProfile()
	end,
	AfterSelectProfile = function()
		if ( THEME:GetMetric("Common","AutoSetStyle") == true ) then
			if IsNetConnected() then
				-- use SelectStyle in online...
				return "ScreenSelectStyle"
			else
				return "ScreenSelectPlayMode"
			end
		else
			return "ScreenSelectStyle"
		end
	end,
	AfterSelectPlayMode = function()
		if IsNetConnected() then
			ReportStyle()
			GAMESTATE:ApplyGameCommand("playmode,regular")
		end
		if IsNetSMOnline() then
			return SMOnlineScreen()
		end
		if IsNetConnected() then
			return "ScreenNetRoom"
		end
		return "ScreenSelectPlayMode"
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
		bTrue = PREFSMAN:GetPreference("ShowInstructions")
		return (bTrue and GoToMusic() or "ScreenGameInformation")
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
		local pm = GAMESTATE:GetPlayMode()
		local restricted = { "PlayMode_Oni", "PlayMode_Rave",
			--"PlayMode_Battle" -- ??
		}
		local optionsScreen = "ScreenPlayerOptions"
		for i=1,#restricted do
			if restricted[i] == pm then
				optionsScreen = "ScreenPlayerOptionsRestricted"
			end
		end
		if SCREENMAN:GetTopScreen():GetGoToOptions() then
			return optionsScreen
		else
			return "ScreenStageInformation"
		end
	end,
	SongOptions = function()
		if SCREENMAN:GetTopScreen():GetGoToOptions() then
			return "ScreenSongOptions"
		else
			return "ScreenStageInformation"
		end
	end,
	GameplayScreen = function()
		if IsRoutine() then
			return "ScreenGameplayShared"
		end
		return "ScreenGameplay"
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
 	AfterSaveSummary = function()
		return GAMESTATE:AnyPlayerHasRankingFeats() and "ScreenNameEntryTraditional" or "ScreenGameOver"
	end,
}
