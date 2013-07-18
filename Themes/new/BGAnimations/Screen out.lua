local t = Def.ActorFrame {};
--
t[#t+1] = Def.ActorFrame {
	InitCommand=cmd(Center);
	--
	Def.Quad {
		InitCommand=cmd(zoomto,SCREEN_WIDTH,SCREEN_HEIGHT;diffuse,Color.Black;diffusealpha,0);
		StartTransitioningCommand=cmd(decelerate,0.25;diffusealpha,1);
	};
};
--
return t