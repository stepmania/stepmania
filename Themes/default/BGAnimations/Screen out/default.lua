local fSleepTime = THEME:GetMetric( Var "LoadingScreen","ScreenOutDelay");
return Def.ActorFrame {
	OnCommand=cmd(sleep,0.15+fSleepTime);
	Def.Quad {
		InitCommand=cmd(Center;zoomto,SCREEN_WIDTH+1,SCREEN_HEIGHT);
		OnCommand=cmd(diffuse,color("0,0,0,0");sleep,fSleepTime;linear,0.15;diffusealpha,1);
	};
};