local t = Def.ActorFrame {};
--
t[#t+1] = Def.ActorFrame {
	InitCommand=cmd(Center);
	--
	Def.Quad {
		InitCommand=cmd(zoomto,SCREEN_WIDTH,SCREEN_HEIGHT;diffuse,Color.Black;diffusealpha,0);
		StartTransitioningCommand=cmd(decelerate,0.125;diffusealpha,1);
	};
	-- Sounds
	LoadActor(THEME:GetPathS(Var "LoadingScreen","cancel")) .. {
		StartTransitioningCommand=cmd(play);
	};
};
--
return t