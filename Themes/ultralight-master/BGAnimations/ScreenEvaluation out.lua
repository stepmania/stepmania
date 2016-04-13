return Def.Quad{
	InitCommand=cmd(FullScreen;diffuse,HSV(192,1,0.05);diffusebottomedge,HSV(192,0.75,0.125);croptop,1;fadetop,1);
	OnCommand=cmd(linear,0.25;croptop,0;fadetop,0;);
};