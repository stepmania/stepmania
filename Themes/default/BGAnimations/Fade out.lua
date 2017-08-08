return Def.ActorFrame {
	StartTransitioningCommand=cmd(sleep,0.15);
	Def.Quad {
		InitCommand=cmd(Center;zoomto,SCREEN_WIDTH+1,SCREEN_HEIGHT;draworder,10000);
		StartTransitioningCommand=cmd(diffusealpha,0;diffuse,color("0,0,0,0");linear,0.3;diffusealpha,1);
	};
};
