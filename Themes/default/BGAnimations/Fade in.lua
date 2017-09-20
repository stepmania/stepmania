local fSleepTime = THEME:GetMetric( Var "LoadingScreen","ScreenOutDelay");
return Def.ActorFrame {
	StartTransitioningCommand=cmd(sleep,0.15+fSleepTime);
	Def.Quad {
		InitCommand=cmd(Center;zoomto,SCREEN_WIDTH+1,SCREEN_HEIGHT;draworder,10000);
		StartTransitioningCommand=cmd(diffuse,color("0,0,0,0");diffusealpha,1;sleep,fSleepTime;linear,0.3;diffusealpha,0);
	};
};
