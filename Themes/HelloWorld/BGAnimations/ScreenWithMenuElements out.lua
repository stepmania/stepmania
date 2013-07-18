return Def.ActorFrame{
	Def.Quad{
		InitCommand=cmd(FullScreen;diffuse,color("#fffdf200"));
		OnCommand=cmd(decelerate,0.875;diffusealpha,1);
	};
}