return Def.ActorFrame {
	Def.Quad {
		InitCommand=cmd(Center;zoomto,SCREEN_WIDTH,SCREEN_HEIGHT);
		OnCommand=cmd(diffuse,color("0,0,0,0");sleep,0.15;linear,0.35;diffusealpha,1);
	};
};