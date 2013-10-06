local t = Def.ActorFrame {};
t[#t+1] = Def.ActorFrame {
	InitCommand=cmd(y,-64);
	Def.Quad {
		InitCommand=cmd(vertalign,top;zoomto,SCREEN_WIDTH,64);
		OnCommand=cmd(diffuse,Color.Black;diffusealpha,0.75);
	};
};
t[#t+1] = Def.ActorFrame {
	InitCommand=cmd(y,-64);
	Def.Quad {
		InitCommand=cmd(vertalign,top;zoomto,SCREEN_WIDTH,48);
		OnCommand=cmd(diffuse,Color.Orange;diffusealpha,0.125;fadebottom,1);
	};
	Def.Quad {
		InitCommand=cmd(vertalign,top;zoomto,SCREEN_WIDTH,2);
		OnCommand=cmd(diffuse,Color.Orange);
	};
};
return t
