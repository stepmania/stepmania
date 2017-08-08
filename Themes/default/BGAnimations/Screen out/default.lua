return Def.ActorFrame {
	StartTransitioningCommand=cmd(sleep,0.7);
	Def.Quad {
		InitCommand=cmd(Center;zoomto,SCREEN_WIDTH+1,SCREEN_HEIGHT;draworder,10000);
		StartTransitioningCommand=cmd(diffusealpha,0;diffuse,color("0,0,0,0");sleep,0.6;linear,0.15;diffusealpha,0);
	};
};
