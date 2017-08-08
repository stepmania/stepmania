local fSleepTime = THEME:GetMetric( Var "LoadingScreen","ScreenInDelay");

return Def.ActorFrame {
	Def.Quad {
		InitCommand=cmd(Center;zoomto,SCREEN_WIDTH+1,SCREEN_HEIGHT);
		OnCommand=cmd(diffuse,color("0,0,0,1");sleep,fSleepTime;linear,0.01;diffusealpha,0);
	};
};