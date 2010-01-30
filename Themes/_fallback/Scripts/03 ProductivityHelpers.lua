-- ProductivityHelpers: A set of useful aliases for theming.
-- This is the sm-ssc version. You should not be using this in themes for
-- SM4 right now... We'll post an updated version soon.

--[[ Aliases ]]

-- Blend Modes
-- Aliases for blend modes.
Blend = {
	Normal   = 'BlendMode_Normal',
	Add      = 'BlendMode_Add',
	Multiply = 'BlendMode_WeightedMultiply',
	Invert   = 'BlendMode_InvertDest',
	NoEffect = 'BlendMode_NoEffect'
};

-- Health Declarations
-- Used primarily for lifebars.
Health = {
	Max    = 'HealthState_Hot',
	Alive  = 'HealthState_Alive',
	Danger = 'HealthState_Danger',
	Dead   = 'HealthState_Dead'
};

--[[ Actor commands ]]
function Actor:CenterX()
	self:x(SCREEN_CENTER_X);
end;

function Actor:CenterY()
	self:y(SCREEN_CENTER_Y);
end;

-- xy(actorX,actorY)
-- Sets the x and y of an actor in one command.
function Actor:xy(actorX,actorY)
	self:x(actorX);
	self:y(actorY);
end;

-- MaskSource()
-- Sets an actor up as the source for a mask.
-- todo: pass in a variable that states whether or not to clearzbuffer
function Actor:MaskSource()
	self:clearzbuffer(true);
	self:zwrite(true);
	self:blend('BlendMode_NoEffect');
end;

-- MaskDest()
-- Sets an actor up to be masked by anything with MaskSource().
function Actor:MaskDest()
	self:ztest(true);
end;

-- Thump()
-- A customized version of pulse that is more appealing for on-beat
-- effects;
function Actor:thump(fEffectPeriod)
	self:pulse()
	if fEffectPeriod ~= nil then
		self:effecttiming(0,0,0.75*fEffectPeriod,0.25*fEffectPeriod);
	else
		self:effecttiming(0,0,0.75,0.25);
	end
	-- The default effectmagnitude will make this effect look very bad.
	self:effectmagnitude(1,1.125,1);
end;

--[[ BitmapText commands ]]

-- PixelFont()
-- An alias that turns off texture filtering.
-- Named because it works best with pixel fonts.
function BitmapText:PixelFont()
	self:SetTextureFiltering(false);
end;

-- Stroke(color)
-- Sets the text's stroke color.
function BitmapText:Stroke(c)
	self:strokecolor( c );
end;

-- NoStroke()
-- Removes any stroke.
function BitmapText:NoStroke()
	self:strokecolor( color("0,0,0,0") );
end;

-- Set Text With Format (contributed by Daisuke Master)
function BitmapText:settextf(...)
	self:settext(string.format(...))
end

-- DiffuseAndStroke(diffuse,stroke)
-- Set diffuse and stroke at the same time.
function BitmapText:DiffuseAndStroke(diffuseC,strokeC)
	self:diffuse(diffuseC);
	self:strokecolor(strokeC);
end;
--[[ end BitmapText commands ]]

--[[ ----------------------------------------------------------------------- ]]

--[[ helper functions ]]
function tobool(v)
	if type(v) == "string" then
		local cmp = string.lower(v);
		if cmp == "true" or cmp == "t" then
			return true;
		elseif cmp == "false" or cmp == "f" then
			return false;
		end;
	elseif type(v) == "number" then
		if v == 0 then return false;
		else return true;
		end;
	end;
end;

function pname(pn) return ToEnumShortString(pn); end;
--[[ end helper functions ]]
-- this code is in the public domain.

--[[ Lists & Functions (sm-ssc specific code) ]]

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

function HoldTiming()
	if GAMESTATE:GetCurrentGame():GetName() == "pump" then
		return "0";
	else return PREFSMAN:GetPreference("TimingWindowSecondsHold");
	end;
end;

function HoldJudgmentFail()
	if GAMESTATE:GetCurrentGame():GetName() == "pump" then
		return "";
	else return "shadowlength,0;diffusealpha,1;zoom,1;y,-10;linear,0.8;y,10;sleep,0.5;linear,0.1;zoomy,0.5;zoomx,2;diffusealpha,0";
	end;
end;

function HoldJudgmentPass()
	if GAMESTATE:GetCurrentGame():GetName() == "pump" then
		return "";
	else return "shadowlength,0;diffusealpha,1;zoom,1.25;linear,0.3;zoomx,1;zoomy,1;sleep,0.5;linear,0.1;zoomy,0.5;zoomx,2;diffusealpha,0";
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