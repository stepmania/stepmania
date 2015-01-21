-- sm-ssc fallback theme | script ring 03 | Gameplay.lua
-- [en] This file is used to store settings that should be different in each
-- game mode.

-- shakesoda calls this pump.lua
local function CurGameName()
	return GAMESTATE:GetCurrentGame():GetName()
end

-- Check the active game mode against a string. Cut down typing this in metrics.
function IsGame(str) return CurGameName():lower() == str:lower() end

-- GetExtraColorThreshold()
-- [en] returns the difficulty threshold in meter
-- for songs that should be counted as boss songs.
function GetExtraColorThreshold()
	local Modes = {
		dance = 10,
		pump = 21,
		beat = 12,
		kb7 = 10,
		para = 10,
		techno = 10,
		lights = 10, -- lights shouldn't be playable
		kickbox= 100, -- extra color is lame
	}
	return Modes[CurGameName()] or 10
end

-- GameplayMargins exists to provide a layer of backwards compatibility for
-- people using the X position metrics to set where the notefields are.
-- This makes it somewhat complex.
-- Rather than trying to understand how it works, you can simply do this:
-- (example values in parentheses)
-- 1.  Decide how much space you want in the center between notefields. (80)
-- 2.  Decide how much space you want on each side. (40)
-- 3.  Write a simple function that just returns those numbers:
--     function GameplayMargins() return 40, 80, 40 end
-- Then the engine does the work of figuring out where each notefield should
-- be centered.
function GameplayMargins(enabled_players, styletype)
	local other= {[PLAYER_1]= PLAYER_2, [PLAYER_2]= PLAYER_1}
	local margins= {[PLAYER_1]= {40, 40}, [PLAYER_2]= {40, 40}}
	-- Use a fake style width because calculating the real style width throws off
	-- the code in the engine.
	local fake_style_width= 272
	-- Handle the case of a single player that is centered first because it's
	-- simpler.
	if Center1Player() then
		local pn= enabled_players[1]
		fake_style_width= 544
		local center= _screen.cx
		local left= center - (fake_style_width / 2)
		local right= _screen.w - center - (fake_style_width / 2)
		-- center margin width will be ignored.
		return left, 80, right
	end
	local half_screen= _screen.w / 2
	local left= {[PLAYER_1]= 0, [PLAYER_2]= half_screen}
	for i, pn in ipairs(enabled_players) do
		local edge= left[pn]
		local center= THEME:GetMetric("ScreenGameplay",
			"Player"..ToEnumShortString(pn)..ToEnumShortString(styletype).."X")
		-- Adjust for the p2 center being on the right side.
		center= center - edge
		margins[pn][1]= center - (fake_style_width / 2)
		margins[pn][2]= half_screen - center - (fake_style_width / 2)
		if #enabled_players == 1 then
			margins[other[pn]][1]= margins[pn][2]
			margins[other[pn]][2]= margins[pn][1]
		end
	end
	local left= margins[PLAYER_1][1]
	local center= margins[PLAYER_1][2] + margins[PLAYER_2][1]
	local right= margins[PLAYER_2][2]
	return left, center, right
end

-- AllowOptionsMenu()
-- [en] returns if you are able to select options
-- on ScreenSelectMusic.
function AllowOptionsMenu()
  if GAMESTATE:IsAnExtraStage() then
    return false
  elseif GAMESTATE:GetPlayMode() == "PlayMode_Oni" then 
    return false
  else
    return true
  end
end

-- GameCompatibleModes:
-- [en] returns possible modes for ScreenSelectPlayMode
function GameCompatibleModes()
	local Modes = {
		dance = "Single,Double,Solo,Versus,Couple",
		pump = "Single,Double,HalfDouble,Versus,Couple,Routine",
		beat = "5Keys,7Keys,10Keys,14Keys,Versus5,Versus7",
		kb7 = "KB7",
		para = "Single",
		maniax = "Single,Double,Versus",
		-- todo: add versus modes for technomotion
		techno = "Single4,Single5,Single8,Double4,Double5,Double8",
		lights = "Single", -- lights shouldn't be playable
	}
	return Modes[CurGameName()]
end

local function upper_first_letter(s)
	local first_letter= s:match("([a-zA-Z])")
	return s:gsub(first_letter, first_letter:upper(), 1)
end

-- No more having a metric for every style for every game mode. -Kyz
function ScreenSelectStyleChoices()
	local styles= GAMEMAN:GetStylesForGame(GAMESTATE:GetCurrentGame():GetName())
	local choices= {}
	for i, style in ipairs(styles) do
		local name= style:GetName()
		local cap_name= upper_first_letter(name)
		-- couple-edit and threepanel don't seem like they should actually be
		-- selectable. -Kyz
		if name ~= "couple-edit" and name ~= "threepanel" then
			choices[#choices+1]= "name," .. cap_name .. ";style," .. name ..
				";text," .. cap_name .. ";screen," .. Branch.AfterSelectStyle()
		end
	end
	return choices
end

-- No more having an xy for every style for every game mode. -Kyz
function ScreenSelectStylePositions(count)
	local poses= {}
	local columns= 1
	local choice_height= 96
	local column_x= {_screen.cx, _screen.cx + 160}
	if count > 4 then
		column_x[1]= _screen.cx - 160
		columns= 2
	end
	if count > 8 then
		column_x[1]= _screen.cx - 240
		column_x[2]= _screen.cx
		column_x[3]= _screen.cx + 240
		columns= 3
	end
	local num_per_column= {math.ceil(count/columns), math.floor(count/columns)}
	if count > 8 then
		if count % 3 == 0 then
			num_per_column[3]= count/columns
		elseif count % 3 == 1 then
			num_per_column[3]= num_per_column[2]
		else
			num_per_column[3]= num_per_column[1]
		end
	end
	for c= 1, columns do
		local start_y= _screen.cy - (choice_height * ((num_per_column[c] / 2)+.5))
		for i= 1, num_per_column[c] do
			poses[#poses+1]= {column_x[c], start_y + (choice_height * i)}
		end
	end
	return poses
end

function SelectProfileKeys()
	local sGame = CurGameName()
	if sGame == "pump" then
		return "Up,Down,Start,Back,Center,DownLeft,DownRight"
	elseif sGame == "dance" then
		return "Up,Down,Start,Back,Up2,Down2"
	else
		return "Up,Down,Start,Back"
	end
end

-- ScoreKeeperClass:
-- [en] Determines the correct ScoreKeeper class to use.
function ScoreKeeperClass()
	-- rave scorekeeper
	if GAMESTATE:GetPlayMode() == 'PlayMode_Rave' then return "ScoreKeeperRave" end
	if GAMESTATE:GetCurrentStyle() then
		local styleType = GAMESTATE:GetCurrentStyle():GetStyleType()
		if styleType == 'StyleType_TwoPlayersSharedSides' then return "ScoreKeeperShared" end
	end
	return "ScoreKeeperNormal"
end

-- ComboContinue:
-- [en] 
function ComboContinue()
	local Continue = {
		dance = GAMESTATE:GetPlayMode() == "PlayMode_Oni" and "TapNoteScore_W2" or "TapNoteScore_W3",
		pump = "TapNoteScore_W3",
		beat = "TapNoteScore_W3",
		kb7 = "TapNoteScore_W3",
		para = "TapNoteScore_W4"
	}
  return Continue[CurGameName()] or "TapNoteScore_W3"
end

function ComboMaintain()
	local Maintain = {
		dance = "TapNoteScore_W3",
		pump = "TapNoteScore_W4",
		beat = "TapNoteScore_W3",
		kb7 = "TapNoteScore_W3",
		para = "TapNoteScore_W4"
	}
  return Maintain[CurGameName()] or "TapNoteScore_W3"
end

function ComboPerRow()
	sGame = CurGameName()
	if sGame == "pump" then
		return true
	elseif GAMESTATE:GetPlayMode() == "PlayMode_Oni" then
		return true
	else
		return false
	end
end

function EvalUsesCheckpointsWithJudgments()
	return (CurGameName() == "pump") and true or false
end

local ComboThresholds = {
	dance	= { Hit = 2, Miss = 2, Fail = -1 },
	pump	= { Hit = 4, Miss = 4, Fail = 51 },
	beat	= { Hit = 1, Miss = 0, Fail = -1 },
	kb7		= { Hit = 1, Miss = 0, Fail = -1 },
	para	= { Hit = 2, Miss = 0, Fail = -1 },
	maniax	= { Hit = 5, Miss = 0, Fail = -1 },
	-------------------------------------------
	default	= { Hit = 2, Miss = 2, Fail = -1 }
}

function HitCombo()
	if ComboThresholds[CurGameName()] then
		return ComboThresholds[CurGameName()].Hit
	end
	return ComboThresholds["default"].Hit
end

function MissCombo()
	if ComboThresholds[CurGameName()] then
		return ComboThresholds[CurGameName()].Miss
	end
	return ComboThresholds["default"].Miss
end

-- FailCombo:
-- [en] The combo that causes game failure.
function FailCombo()
	-- ITG (dance) uses 30. Pump Pro uses 30, real Pump uses 51
	if ComboThresholds[CurGameName()] then
		return ComboThresholds[CurGameName()].Fail
	end
	return ComboThresholds["default"].Fail
end

function TwoPartSelection()
	return GAMESTATE:GetCurrentGame():GetName() == "pump" and true or false 
end 

local RoutineSkins = {
	dance	= { P1 = "midi-routine-p1", P2 = "midi-routine-p2" },
	pump	= { P1 = "cmd-routine-p1", P2 = "cmd-routine-p2" },
	kb7		= { P1 = "default", P2 = "retrobar" },
	-------------------------------------------------------------
	default	= { P1 = "default", P2 = "default" }
}

function RoutineSkinP1()
	if RoutineSkins[CurGameName()] then
		return RoutineSkins[CurGameName()].P1
	end
	return RoutineSkins["default"].P1
end

function RoutineSkinP2()
	if RoutineSkins[CurGameName()] then
		return RoutineSkins[CurGameName()].P2
	end
	return RoutineSkins["default"].P2
end

-- todo: use tables for some of these -aj
function HoldTiming()
	return IsGame("pump") and 0 or PREFSMAN:GetPreference("TimingWindowSecondsHold")
end

function ShowHoldJudgments()
	return not IsGame("pump")
end

local CodeDetectorCodes = {
	-- steps
	PrevSteps1 = {
		default = "",
		dance = "Up,Up",
		pump = "+UpLeft",
	},
	PrevSteps2 = {
		default = "MenuUp,MenuUp",
		dance = "MenuUp,MenuUp",
		pump = "",
	},
	NextSteps1 = {
		default = "",
		dance = "Down,Down",
		pump = "+UpRight",
	},
	NextSteps2 = {
		default = "MenuDown,MenuDown",
		dance = "MenuDown,MenuDown",
		pump = "",
	},
	-- group
	NextGroup = {
		default = "",
		dance = "MenuUp,MenuRight,MenuRight",
		pump = "",
	},
	PrevGroup = {
		default = "",
		dance = "MenuUp,MenuDown,MenuUp,MenuDown",
		pump = "",
	},
	CloseCurrentFolder = {
		default = "MenuUp-MenuDown",
	},
	-- sorts
	NextSort1 = {
		default = "@MenuLeft-@MenuRight-Start",
		dance = "@MenuLeft-@MenuRight-Start",
		pump = "@MenuLeft-@MenuRight-Start",
	},
	NextSort2 = {
		default = "MenuLeft-MenuRight",
		dance = "MenuLeft-MenuRight",
		pump = "MenuLeft-MenuRight",
	},
	NextSort3 = {
		default = "",
		dance = "@Left-@Right-Start",
		pump = "@DownLeft-@DownRight-Start",
	},
	NextSort4 = {
		default = "",
		dance = "Left-Right",
		pump = "DownLeft-DownRight",
	},
	-- modemenu
	ModeMenu1 = {
		default = "",
		dance = "Up,Down,Up,Down",
	},
	ModeMenu2 = {
		default = "MenuUp,MenuDown,MenuUp,MenuDown",
	},
	-- Evaluation:
	SaveScreenshot1 = {
		default = "MenuLeft-MenuRight",
	},
	SaveScreenshot2 = {
		default = "Select",
	},
	-- modifiers section
	CancelAll = {
		default = "",
		dance = "Left,Right,Left,Right,Left,Right,Left,Right",
	},
	--- specific modifiers
	Mirror = {
		default = "",
		dance = "",
		pump = "DownRight,DownLeft,UpRight,UpLeft,DownRight,DownLeft,UpRight,UpLeft,Center",
	},
	Left = {
		default = "",
		dance = "",
	},
	Right = {
		default = "",
		dance = "",
	},
	Shuffle = {
		default = "",
		dance = "",
		pump = "UpLeft,UpRight,UpLeft,UpRight,DownLeft,DownRight,DownLeft,DownRight,Center", -- random
	},
	SuperShuffle = {
		default = "",
		dance = "",
		pump = "UpLeft,UpRight,DownLeft,DownRight,UpLeft,UpRight,DownLeft,DownRight,Center"
	},
	Reverse = {
		default = "",
		dance = "",
		pump = "UpLeft,DownLeft,UpRight,DownRight,UpLeft,DownLeft,UpRight,DownRight,DownRight", -- drop
	},
	HoldNotes = {
		default = "",
		dance = "",
	},
	Mines = {
		default = "",
	},
	Dark = {
		default = "",
	},
	Hidden = {
		default = "",
		pump = "UpLeft,UpRight,DownLeft,DownRight,Center", -- vanish
	},
	RandomVanish = {
		default = "",
	},
	-- boost (accel), brake (decel), stealth (nonstep)
	--- next/prev modifiers
	NextTransform = {
		default = "",
	},
	NextScrollSpeed = {
		default = "",
		dance = "",
		pump = "UpLeft,UpRight,UpLeft,UpRight,Center",
	},
	PreviousScrollSpeed = {
		default = "",
		dance = "",
		pump = "UpRight,UpLeft,UpRight,UpLeft,Center",
	},
	NextAccel = {
		default = "",
		dance = "",
	},
	NextEffect = {
		default = "",
		dance = "",
	},
	NextAppearance = {
		default = "",
		dance = "",
	},
	NextTurn = {
		default = "",
	},
	-- cancel all in player options
	CancelAllPlayerOptions = {
		default = "",
		dance = "Left,Right,Left,Right,Left,Right",
	},
};

function GetCodeForGame(codeName)
	local gameName = string.lower(CurGameName())
	local inputCode = CodeDetectorCodes[codeName]
	return inputCode[gameName] or inputCode["default"]
end
