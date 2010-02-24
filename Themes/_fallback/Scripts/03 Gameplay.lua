-- sm-ssc fallback theme | script ring 03 | Gameplay.lua
-- [en] This file is used to store settings that should be different in each
-- game mode.

-- GameCompatibleModes:
-- [en] returns possible modes for ScreenSelectPlayMode
function GameCompatibleModes()
	sGame = GAMESTATE:GetCurrentGame():GetName();
	local Modes = {
		dance = "Single,Double,Solo,Versus,Couple",
		pump = "Single,Double,HalfDouble,Versus,Couple",
		beat = "5Keys,7Keys,10Keys,14Keys",
		kb7 = "KB7",
		para = "Single",
		lights = "Single", -- lights shouldn't be playable
	};
	return Modes[sGame];
end

-- ComboContinue:
-- [en] 
function ComboContinue()
	sGame = GAMESTATE:GetCurrentGame():GetName();
	local Continue = {
		dance = GAMESTATE:GetPlayMode() == "PlayMode_Oni" and "TapNoteScore_W2" or "TapNoteScore_W3",
		pump = "TapNoteScore_W4",
		beat = "TapNoteScore_W3",
		kb7 = "TapNoteScore_W3",
		para = "TapNoteScore_W4",
	};
	return Continue[sGame]
end;

function ComboMaintain()
	sGame = GAMESTATE:GetCurrentGame():GetName();
	local Maintain = {
		dance = "TapNoteScore_W3",
		pump = "TapNoteScore_W4",
		beat = "TapNoteScore_W3",
		kb7 = "TapNoteScore_W3",
		para = "TapNoteScore_W4",
	};
	return Maintain[sGame]
end;

function ComboPerRow()
	sGame = GAMESTATE:GetCurrentGame():GetName();
	if sGame == "pump" then
		return true;
	elseif GAMESTATE:GetPlayMode() == "PlayMode_Oni" then
		return true;
	else return false;
	end;
end;

function HitCombo()
	sGame = GAMESTATE:GetCurrentGame():GetName();
	local Combo = {
		dance = 2,
		pump = 4,
		beat = 2,
		kb7 = 2,
		para = 2,
	};
	return Combo[sGame]
end;

function MissCombo()
	sGame = GAMESTATE:GetCurrentGame():GetName();
	local Combo = {
		dance = 2,
		pump = 4,
		beat = 0,
		kb7 = 0,
		para = 0,
	};
	return Combo[sGame]
end;

function FailCombo() -- The combo that causes game failure.
	sGame = GAMESTATE:GetCurrentGame():GetName();
	local Combo = {
		dance = 30, -- ITG/Pump Pro does it this way.
		pump = 51,
		beat = -1,
		kb7 = -1,
		para = -1,
	};
	return Combo[sGame]
end;

-- todo: use tables for some of these -aj
function HoldTiming()
	if GAMESTATE:GetCurrentGame():GetName() == "pump" then
		return 0;
	else return PREFSMAN:GetPreference("TimingWindowSecondsHold");
	end;
end;

function HoldJudgmentFail()
	if GAMESTATE:GetCurrentGame():GetName() == "pump" then
		return cmd();
	else return cmd(finishtweening;shadowlength,0;diffusealpha,1;zoom,1;y,-10;linear,0.8;y,10;sleep,0.5;linear,0.1;zoomy,0.5;zoomx,2;diffusealpha,0);
	end;
end;

function HoldJudgmentPass()
	if GAMESTATE:GetCurrentGame():GetName() == "pump" then
		return cmd();
	else return cmd(finishtweening;shadowlength,0;diffusealpha,1;zoom,1.25;linear,0.3;zoomx,1;zoomy,1;sleep,0.5;linear,0.1;zoomy,0.5;zoomx,2;diffusealpha,0);
	end;
end;

function HoldHeadStep()
	if GAMESTATE:GetCurrentGame():GetName() == "pump" then
		return false;
	else return true;
	end;
end;

function InitialHoldLife()
	if GAMESTATE:GetCurrentGame():GetName() == "pump" then
		return 0.05;
	else return 1;
	end;
end;

function MaxHoldLife()
	if GAMESTATE:GetCurrentGame():GetName() == "pump" then
		return 0.05;
	else return 1;
	end;
end;

function ImmediateHoldLetGo()
	if GAMESTATE:GetCurrentGame():GetName() == "pump" then
		return false;
	else return true;
	end;
end;

function RollBodyIncrementsCombo()
	if GAMESTATE:GetCurrentGame():GetName() == "pump" then
		return false;
	else return true;
	end;
end;

function CheckpointsTapsSeparateJudgment()
	if GAMESTATE:GetCurrentGame():GetName() == "pump" then
		return false;
	else return true;
	end;
end;

function ScoreMissedHoldsAndRolls()
	if GAMESTATE:GetCurrentGame():GetName() == "pump" then
		return false;
	else return true;
	end;
end;