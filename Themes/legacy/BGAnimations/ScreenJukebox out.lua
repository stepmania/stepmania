return Def.ActorFrame{
	Def.Quad{
		InitCommand=cmd(FullScreen;diffuse,color("0,0,0,0"));
		OnCommand=cmd(accelerate,0.5;diffusealpha,1);
	};
};