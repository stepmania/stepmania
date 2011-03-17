return Def.ActorFrame{
	Def.Quad{
		InitCommand=cmd(FullScreen;diffuse,color("#fffdf200"));
		OnCommand=cmd(sleep,1.125;decelerate,0.875;diffusealpha,1);
	};
}