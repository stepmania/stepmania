-- sm-ssc fallback theme | script ring 03 | Gameplay.lua
-- [en] This file is used to store settings that should be different in each
-- game mode.

-- shakesoda calls this pump.lua

-- GetEditModifiers
-- [en] returns modifiers for ScreenEdit to use.
function GetEditModifiers()
	sGame = GAMESTATE:GetCurrentGame():GetName()
	local Modes = {
		dance = "",
		pump = "",
		beat = "reverse",
		kb7 = "reverse",
		para = "",
		techno = "",
		lights = "" -- lights shouldn't be playable
	}
	return Modes[sGame]
end

-- GameCompatibleModes:
-- [en] returns possible modes for ScreenSelectPlayMode
function GameCompatibleModes()
	sGame = GAMESTATE:GetCurrentGame():GetName()
	local Modes = {
		dance = "Single,Double,Solo,Versus,Couple",
		pump = "Single,Double,HalfDouble,Versus,Couple",
		beat = "5Keys,7Keys,10Keys,14Keys",
		kb7 = "KB7",
		para = "Single",
		techno = "Single4,Single5,Single8,Double4,Double8",
		lights = "Single" -- lights shouldn't be playable
	}
	return Modes[sGame]
end

function SelectProfileKeys()
	local game = GAMESTATE:GetCurrentGame():GetName()
	return game == "dance" and "Up,Down,Start,Back,Up2,Down2" or "Up,Down,Start,Back"
end

-- ScoreKeeperClass:
-- [en] Determines the correct ScoreKeeper class to use.
function ScoreKeeperClass()
	sGame = GAMESTATE:GetCurrentGame():GetName()
	local ScoreKeepers = {
		-- xxx: allow for ScoreKeeperShared when needed
		dance = "ScoreKeeperNormal",
		pump = "ScoreKeeperNormal",
		beat = "ScoreKeeperNormal",
		kb7 = "ScoreKeeperNormal",
		para = "ScoreKeeperNormal",
		techno = "ScoreKeeperNormal",
		ez2 = "ScoreKeeperNormal",
		ds3ddx = "ScoreKeeperNormal",
		maniax = "ScoreKeeperNormal",
		guitar = "ScoreKeeperGuitar"
	}
	return ScoreKeepers[sGame]
end

-- ComboContinue:
-- [en] 
function ComboContinue()
	sGame = GAMESTATE:GetCurrentGame():GetName()
	local Continue = {
		dance = GAMESTATE:GetPlayMode() == "PlayMode_Oni" and "TapNoteScore_W2" or "TapNoteScore_W3",
		pump = "TapNoteScore_W4",
		beat = "TapNoteScore_W3",
		kb7 = "TapNoteScore_W3",
		para = "TapNoteScore_W4"
	}
	return Continue[sGame]
end

function ComboMaintain()
	sGame = GAMESTATE:GetCurrentGame():GetName()
	local Maintain = {
		dance = "TapNoteScore_W3",
		pump = "TapNoteScore_W4",
		beat = "TapNoteScore_W3",
		kb7 = "TapNoteScore_W3",
		para = "TapNoteScore_W4"
	}
	return Maintain[sGame]
end

function ComboPerRow()
	sGame = GAMESTATE:GetCurrentGame():GetName()
	if sGame == "pump" then
		return true
	elseif GAMESTATE:GetPlayMode() == "PlayMode_Oni" then
		return true
	else
		return false
	end
end

-- these need cleanup really.
function HitCombo()
	sGame = GAMESTATE:GetCurrentGame():GetName()
	local Combo = {
		dance = 2,
		pump = 4,
		beat = 2,
		kb7 = 2,
		para = 2,
		guitar = 2
	}
	return Combo[sGame]
end

function MissCombo()
	sGame = GAMESTATE:GetCurrentGame():GetName()
	local Combo = {
		dance = 2,
		pump = 4,
		beat = 0,
		kb7 = 0,
		para = 0,
		guitar = 0
	}
	return Combo[sGame]
end

-- FailCombo:
-- [en] The combo that causes game failure.
function FailCombo()
	sGame = GAMESTATE:GetCurrentGame():GetName()
	local Combo = {
		dance = -1, -- ITG uses 30
		pump = 51, -- Pump Pro uses 30, real Pump uses 51
		beat = -1,
		kb7 = -1,
		para = -1,
		guitar = -1
	}
	return Combo[sGame]
end

-- todo: use tables for some of these -aj
function HoldTiming()
	if GAMESTATE:GetCurrentGame():GetName() == "pump" then
		return 0
	else
		return PREFSMAN:GetPreference("TimingWindowSecondsHold")
	end
end

function ShowHoldJudgments()
	return not GAMESTATE:GetCurrentGame():GetName() == "pump"
end

function HoldHeadStep()
	if GAMESTATE:GetCurrentGame():GetName() == "pump" then
		return false
	else
		return true
	end
end

function InitialHoldLife()
	if GAMESTATE:GetCurrentGame():GetName() == "pump" then
		return 0.05
	else
		return 1
	end
end

function MaxHoldLife()
	if GAMESTATE:GetCurrentGame():GetName() == "pump" then
		return 0.05
	else
		return 1
	end
end

function ImmediateHoldLetGo()
	if GAMESTATE:GetCurrentGame():GetName() == "pump" then
		return false
	else
		return true
	end
end

function RollBodyIncrementsCombo()
	return false
--[[ 	if GAMESTATE:GetCurrentGame():GetName() == "pump" then
		return false
	else
		return true
	end --]]
end

function CheckpointsTapsSeparateJudgment()
	if GAMESTATE:GetCurrentGame():GetName() == "pump" then
		return false
	else
		return true
	end
end

function ScoreMissedHoldsAndRolls()
	if GAMESTATE:GetCurrentGame():GetName() == "pump" then
		return false
	else
		return true
	end
end

local tNotePositions = {
	-- StepMania 3.9/4.0
	Normal = {
		-144,
		144,
	},
	-- ITG
	Lower = {
		-125,
		145,
	}
}

function GetTapPosition( sType )
	bCategory = (sType == 'Standard') and 1 or 2
	-- true: Normal
	-- false: Lower
	bPreference = GetUserPrefB("UserPrefNotePosition") and "Normal" or "Lower"
	tNotePos = tNotePositions[bPreference]
	return tNotePos[bCategory]
end