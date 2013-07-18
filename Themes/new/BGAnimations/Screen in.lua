local t = Def.ActorFrame {};
--
t[#t+1] = Def.ActorFrame {
	InitCommand=cmd(Center);
	--
	Def.Quad {
		InitCommand=cmd(zoomto,SCREEN_WIDTH,SCREEN_HEIGHT;diffuse,Color.Black;diffusealpha,1);
		StartTransitioningCommand=cmd(smooth,0.2;diffusealpha,0);
	};
};
--
return t