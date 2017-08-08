local t = Def.ActorFrame{};
	t[#t+1] = Def.Quad{
		InitCommand=cmd(FullScreen;diffuse,color("1,0,0,0");blend,Blend.Multiply);
		OnCommand=cmd(smooth,1;diffuse,color("0.75,0,0,0.75");decelerate,2;diffuse,color("0,0,0,1"));
	};
	t[#t+1] = Def.Quad{
		InitCommand=cmd(FullScreen;diffuse,color("1,1,1,1");diffusealpha,0);
		OnCommand=cmd(finishtweening;diffusealpha,1;decelerate,1.25;diffuse,color("1,0,0,0"));
	};
	t[#t+1] = LoadActor(THEME:GetPathS( Var "LoadingScreen", "failed" ) ) .. {
		StartTransitioningCommand=cmd(play);
	};

return t;
