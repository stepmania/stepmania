if not GAMESTATE:IsCourseMode() then return Def.ActorFrame{} end; -- short circuit
local course = GAMESTATE:GetCurrentCourse()

local t = Def.ActorFrame{
	-- background
	Def.Sprite{
		InitCommand=cmd(Center);
		BeginCommand=function(self)
			if course:GetBackgroundPath() then
				self:Load( course:GetBackgroundPath() )
			else
				-- default to the BG of the first song in the course
				self:LoadFromCurrentSongBackground()
			end
		end;
		OnCommand=cmd(diffusealpha,0;scaletoclipped,SCREEN_WIDTH,SCREEN_HEIGHT;sleep,0.5;linear,0.50;diffusealpha,1;sleep,3);
	};
	-- alternate background
	Def.Sprite{
		InitCommand=cmd(Center;);
		BeginCommand=cmd(LoadFromCurrentSongBackground;scaletoclipped,SCREEN_WIDTH,SCREEN_HEIGHT;diffusealpha,0);
		OnCommand=cmd(sleep,4;playcommand,"Show");
		ShowCommand=function(self)
			if course:HasBackground() then
				self:accelerate(0.25)
				self:diffusealpha(1)
			end
		end;
	};
};

return t;