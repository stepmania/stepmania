return Def.ActorFrame{
	Def.Quad{
		InitCommand=cmd(FullScreen;diffuse,color("#fffdf2FF"));
		OnCommand=cmd(decelerate,0.75;diffusealpha,0);
	};
};