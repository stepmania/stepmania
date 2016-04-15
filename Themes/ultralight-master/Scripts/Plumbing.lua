-- here's some stuff I didn't feel like shoving anywhere else
function WideScale(AR4_3, AR16_9) return scale( SCREEN_WIDTH, 640, 854, AR4_3, AR16_9 ) end

function IsPlayerValid(pn)
	local pm = GAMESTATE:GetPlayMode()
	if pm == 'PlayMode_Rave' or pm == 'PlayMode_Battle' then
		-- in rave/battle mode, we may have a computer player.
		return GAMESTATE:IsPlayerEnabled(pn)
	else
		return GAMESTATE:IsHumanPlayer(pn)
	end
	return false
end;

function StageDisplayY()
	local bWidescreen = GetScreenAspectRatio() >= 1.6
	local bSharedLifeMeter = GAMESTATE:GetPlayMode() == 'PlayMode_Rave' or GAMESTATE:GetPlayMode() == 'PlayMode_Battle'
	if not bSharedLifeMeter and bWidescreen then
		return SCREEN_TOP+28
	else
		return SCREEN_TOP+40
	end
end;

function met(section,metricname) return THEME:GetMetric(section,metricname) end

ScreenString = Screen.String
ScreenMetric = Screen.Metric

local gameToStepsTypeIcon = {
		dance = "_dance-single",
		pump = "_pump-single",
		kb7 = "_kb7",
		-- ez2,
		para = "_para",
		-- ds3ddx
		beat = "_beat7",
		maniax = "_maniax-single",
		techno = "_techno8",
		popn = "_popn9",
		lights = "_lights"
	}

function GetGameIcon()
	local curGameName = GAMESTATE:GetCurrentGame():GetName()
	-- show something in case I didn't cover a base
	return gameToStepsTypeIcon[curGameName] or "_base"
end

function GetScoreKeeper()
	local ScoreKeeper = "ScoreKeeperNormal"
	local game = GAMESTATE:GetCurrentGame():GetName()

	-- two players and shared sides need the Shared ScoreKeeper.
	local styleType = GAMESTATE:GetCurrentStyle():GetStyleType()
	if styleType == 'StyleType_TwoPlayersSharedSides' then
		ScoreKeeper = "ScoreKeeperShared"
	end

	return ScoreKeeper
end

function HeaderString(h) return THEME:GetString("Headers",h) end

--[[ * Summary Banners * ]]
-- If this code looks familiar, that's because it is.
-- I'm really lazy and stole it from my foonmix port (and in that theme, I stole
-- it from my DDR 5th port, which I guess makes that the true source of this code)

-- todo: make this ish less bootleg.

local summaryBannerX = {
	MaxStages1 = { SCREEN_CENTER_X },
	MaxStages2 = {
		SCREEN_CENTER_X+45,
		SCREEN_CENTER_X-45
	},
	MaxStages3 = {
		SCREEN_CENTER_X+60,
		SCREEN_CENTER_X,
		SCREEN_CENTER_X-60
	},
	MaxStages4 = {
		SCREEN_CENTER_X+45,
		SCREEN_CENTER_X+15,
		SCREEN_CENTER_X-15,
		SCREEN_CENTER_X-45
	},
	MaxStages5 = {
		SCREEN_CENTER_X+60,
		SCREEN_CENTER_X+30,
		SCREEN_CENTER_X,
		SCREEN_CENTER_X-30,
		SCREEN_CENTER_X-60
	}
}

local summaryBannerY = {
	MaxStages1 = { SCREEN_CENTER_Y*0.3 },
	MaxStages2 = {
		SCREEN_CENTER_Y-130,	-- todo
		SCREEN_CENTER_Y-150		-- todo
	},
	MaxStages3 = {
		SCREEN_CENTER_Y-120,	-- todo
		SCREEN_CENTER_Y*0.3,
		SCREEN_CENTER_Y-160		-- todo
	},
	MaxStages4 = {
		SCREEN_CENTER_Y-125,	-- todo
		SCREEN_CENTER_Y-135,	-- todo
		SCREEN_CENTER_Y-145,	-- todo
		SCREEN_CENTER_Y-155		-- todo
	},
	MaxStages5 = {
		SCREEN_CENTER_Y-120,	-- todo
		SCREEN_CENTER_Y-130,	-- todo
		SCREEN_CENTER_Y*0.3,
		SCREEN_CENTER_Y-150,	-- todo
		SCREEN_CENTER_Y-160		-- todo
	}
}

-- pType is either "X" or "Y"
function GetSummaryBannerPos(pType,num)
	local maxStages = PREFSMAN:GetPreference('SongsPerPlay')
	local t = (pType=="X" and summaryBannerX or summaryBannerY)

	-- check how many stages were played...
	local playedStages = STATSMAN:GetStagesPlayed()
	if playedStages < maxStages then
		-- long versions and/or marathons were involved.
		if playedStages == 1 then return t.MaxStages1[1]
		else
			return t[string.format("MaxStages%d",playedStages)][num]
		end
	elseif playedStages > maxStages then
		-- extra stages
		if playedStages == 1 then return summaryBannerX.MaxStages1[1]
		else
			return t[string.format("MaxStages%d",playedStages)][num]
		end
	else
		-- normal behavior
		if maxStages == 1 then return summaryBannerX.MaxStages1[1]
		else
			return t[string.format("MaxStages%d",maxStages)][num]
		end
	end
end

--[[ * Theme Preferences * ]]
-- This is using the old ThemePrefs system. Converting it to use something else
-- will probably happen in the future, in case ThemePrefs magically goes away somehow.

-- need this for localized strings:
local function OptionNameString(str) return THEME:GetString('OptionNames',str) end

-- theme preferences
local ultralightPrefs = {
	AutoSetStyle = {
		Default = true,
		Choices = { OptionNameString('Off'), OptionNameString('On') },
		Values = { false, true }
	},
	ComboUnderField = {
		Default = true,
		Choices = { OptionNameString('Off'), OptionNameString('On') },
		Values = { false, true }
	},
	GameplayFooter = {
		Default = false,
		Choices = { OptionNameString('Off'), OptionNameString('On') },
		Values = { false, true }
	},
	ReceptorPosition = {
		Default = false,
		Choices = { OptionNameString('Normal'), OptionNameString('ITG') },
		Values = { false, true }
	},
}
ThemePrefs.InitAll(ultralightPrefs)
