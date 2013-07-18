return Def.Quad {
	InitCommand=cmd(diffuse,color("#000000");stretchto,SCREEN_LEFT,SCREEN_TOP,SCREEN_RIGHT,SCREEN_BOTTOM);
	StartTransitioningCommand=cmd(diffusealpha,1;linear,0.3;diffusealpha,0);
};
