local t = Def.ActorFrame {};
t[#t+1] = Def.ActorFrame {
	InitCommand=cmd(y,-64);
	Def.Quad {
		InitCommand=cmd(vertalign,top;zoomto,SCREEN_WIDTH-32,2);
		OnCommand=cmd(diffusealpha,0.5);
	};
};
return t