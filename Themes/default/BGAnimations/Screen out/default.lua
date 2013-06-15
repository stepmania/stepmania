local fSleepTime = THEME:GetMetric( Var "LoadingScreen","ScreenOutDelay");

return Def.ActorFrame {
	Def.Quad {
		InitCommand=cmd(Center;zoomto,SCREEN_WIDTH+1,SCREEN_HEIGHT);
		OnCommand=cmd(diffuse,color("0,0,0,0");sleep,fSleepTime;linear,0.01;diffusealpha,1);
	};
};