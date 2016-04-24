-- xxx: fixme: I don't scale right.
--[[
local t = Def.ActorFrame {
	Def.Sprite{
		InitCommand=cmd(Center);
		BeforeLoadingNextCourseSongMessageCommand=function(self)
			self:LoadFromSongBackground( SCREENMAN:GetTopScreen():GetNextCourseSong() )
		end;
		StartCommand=cmd(scale_or_crop_background;diffusealpha,0;sleep,0.75;decelerate,0.5;diffusealpha,1);
		FinishCommand=cmd(sleep,0.75;accelerate,0.5;diffusealpha,0);
	};
};
--]]

local t = Def.ActorFrame{};

if not GAMESTATE:IsCourseMode() then return t; end;

t[#t+1] = Def.Sprite {
	InitCommand=cmd(Center);
	BeforeLoadingNextCourseSongMessageCommand=function(self) self:LoadFromSongBackground( SCREENMAN:GetTopScreen():GetNextCourseSong() ) end;
	StartCommand=cmd(cropleft,1;fadeleft,1;cropright,0;faderight,0;scaletoclipped,SCREEN_WIDTH,SCREEN_HEIGHT;linear,1;fadeleft,0;cropleft,0;sleep,1);
	FinishCommand=cmd(linear,1;cropright,1;faderight,1;diffusealpha,PREFSMAN:GetPreference("BGBrightness"));
};

return t;