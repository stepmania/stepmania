return Def.ActorFrame {
	Def.Quad{
		InitCommand=cmd(scaletocover,-SCREEN_WIDTH*2,SCREEN_TOP,SCREEN_WIDTH*2,SCREEN_BOTTOM;diffuse,color("0,0,0,0.5"));
		OnCommand=cmd(diffusealpha,0;smooth,0.2;diffusealpha,0.5);
		OffCommand=cmd(smooth,0.2;diffusealpha,0);
	};
};