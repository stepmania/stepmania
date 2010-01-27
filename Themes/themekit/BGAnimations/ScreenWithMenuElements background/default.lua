return Def.ActorFrame {
	InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y);
	
	Def.Quad {
		InitCommand=cmd(zoomto,SCREEN_WIDTH,SCREEN_HEIGHT);
		OnCommand=cmd(diffuse,color("0.25,0.15,0.25,1"));
	};
};