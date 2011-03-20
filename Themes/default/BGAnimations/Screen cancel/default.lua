return Def.ActorFrame {
	Def.Quad {
		InitCommand=cmd(Center;zoomto,SCREEN_WIDTH+1,SCREEN_HEIGHT);
		OnCommand=cmd(diffuse,color("0,0,0,0.5");sleep,0.15;diffusealpha,1;sleep,0.15);
	};
	LoadActor(THEME:GetPathS("_Screen","cancel")) .. {
		StartTransitioningCommand=cmd(play);
	};
};