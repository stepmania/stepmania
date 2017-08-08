return Def.ActorFrame {
	Def.Quad{
		InitCommand=cmd(FullScreen;diffuse,color("1,0,0,0");blend,Blend.Multiply);
		OnCommand=cmd(smooth,1;diffuse,color("0.75,0,0,0.75");decelerate,2;diffuse,color("0,0,0,1"));
	};
};