function StageBreak()
	if GAMESTATE:IsEventMode()
	or GAMESTATE:GetEasiestStepsDifficulty() == 'Difficulty_Beginner'
	or GAMESTATE:GetEasiestStepsDifficulty() == 'Difficulty_Easy'
	then
		return -1
	end
	
	return 51
end

function JudgmentTransformCommand( self, params )
	local y = 0
	
	if params.bCentered then
		if params.bReverse then
			y = 100
		else
			y = -170
		end
	end
	
	self:y(y)
end

function Tracef(...)
	Trace(string.format(...))
end

function DifficultyAndStepstypeToString( difficulty, stepstype, description )
	--safe
	difficulty = string.lower(difficulty)
	stepstype = string.lower(stepstype)
	--
	description = description == "" and "Edit" or description
	if string.find(stepstype,"double") then
		description = description.." - Double"
	else
		description = description.." - Single"
	end
	
	--return {difficulty, stepstype, description}
	
	local strings = {
		stepstype_pump_single = {
			difficulty_beginner =	"Easy";
			difficulty_easy =		"Normal";
			difficulty_medium =		"Hard";
			difficulty_hard =		"Crazy";
			difficulty_challenge =	"Wild";
			difficulty_edit =		description;
		};
		stepstype_pump_double = {
			difficulty_beginner =	"Double";
			difficulty_easy =		"Performance";
			difficulty_medium =		"Freestyle";
			difficulty_hard =		"Nightmare";
			difficulty_challenge =	"Hardcore";
			difficulty_edit =		description;
		};
		stepstype_pump_halfdouble = {
			--figure out more names to identify
			difficulty_beginner =	"Halfdouble";
			difficulty_easy =		"Halfdouble";
			difficulty_medium =		"Halfdouble";
			difficulty_hard =		"Halfdouble";
			difficulty_challenge =	"Halfdouble";
			difficulty_edit =		description;
		};
		other = {
			--touhou, sorry
			difficulty_beginner =	"Easy";	
			difficulty_easy =		"Normal";
			difficulty_medium =		"Hard";
			difficulty_hard =		"Lunatic";
			difficulty_challenge =	"Extra";
			difficulty_edit =		description;
		};
	}
	
	local game = GAMESTATE:GetCurrentGame():GetName()
	local mode = string.gsub(stepstype,game.."_","")
	mode = string.gsub(mode,"stepstype_","")
	
	--if game == mode then mode = "" else mode = "-"..mode end
	
	local FallbackString = string.format("%s-%s %s", game, mode, strings.other[difficulty])
	
	local ret = strings[stepstype]
	if not ret then return FallbackString end
	
	return ret[difficulty] or FallbackString
end
-->> media loaders
function LoadSound( a, b ) return LoadActor( THEME:GetPathS(b and a or "", b or a) ) end;
--<< end media loaders
-->> positioning helpers (handy!)
function Actor:FromCenterX(i)
	self:x(SCREEN_CENTER_X + i)
end

function Actor:FromCenterY(i)
	self:y(SCREEN_CENTER_Y + i)
end

function Actor:FromLeft(i)
	self:x(SCREEN_LEFT + i)
end

function Actor:FromRight(i)
	self:x(SCREEN_RIGHT + i)
end

function Actor:FromTop(i)
	self:y(SCREEN_TOP + i)
end

function Actor:FromBottom(i)
	self:y(SCREEN_BOTTOM + i)
end
--<< end positioning helpers
-->> scale helpers
--like stretchto, but the values are relative to the edges
function Actor:Rect(left, top, right, bottom)
	self:stretchto(left, top, SCREEN_RIGHT - right, SCREEN_BOTTOM - bottom);
end

function Actor:Skew(x,y)
	self:skewx(x);
	self:skewy(x);
end

--I wonder if it's like that...
function Actor:SkewPixels(x,y)
	self:skewx(scale(x,0,self:GetWidth(),0,1));
	self:skewy(scale(y,0,self:GetHeight(),0,1));
end
--<< end scale helpers
-->> frame set helpers
function Sprite.FrameRange(s,f,d)
	local frames = {}
	for i=s,f do
		frames[#frames+1] = {
			Frame = i;
			Delay = d;
		};
	end
	return frames
end
--<< end frame set helpers

--Alberto Ramos
--MIT License