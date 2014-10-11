local t = Def.ActorFrame {
	Def.Quad {
		InitCommand=cmd(zoomto,SCREEN_WIDTH,SCREEN_HEIGHT);
		OnCommand=cmd(diffuse,Color.Black;diffusealpha,0);
		StartSelectingStepsMessageCommand=cmd(stoptweening;linear,0.2;diffusealpha,0.75);
		SongUnchosenMessageCommand=cmd(stoptweening;linear,0.2;diffusealpha,0);
	};
};

t.InitCommand=cmd(Center);

return t;
