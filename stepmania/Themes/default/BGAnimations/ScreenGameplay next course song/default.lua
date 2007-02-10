local children = {
	Def.Quad {
		InitCommand=cmd(diffuse,color("#000000");stretchto,SCREEN_LEFT,SCREEN_TOP,SCREEN_RIGHT,SCREEN_BOTTOM);
		StartCommand=cmd(diffusealpha,0;sleep,1;linear,0.5;diffusealpha,1);
		FinishCommand=cmd(sleep,1;linear,0.5;diffusealpha,0);
	};
	Def.Sprite {
		InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y);
		BeforeLoadingNextCourseSongMessageCommand=function(self) self:LoadFromSongBanner( SCREENMAN:GetTopScreen():GetNextCourseSong() ) end;
		StartCommand=cmd(scaletoclipped,512,160;diffusealpha,0;sleep,1;linear,0.5;diffusealpha,1);
		FinishCommand=cmd(sleep,1;linear,0.5;diffusealpha,0);
	};
};
return Def.ActorFrame { children = children };
