local t = Def.ActorFrame {};
t[#t+1] = Def.ActorFrame {
	Def.Quad {
		InitCommand=cmd(vertalign,bottom;zoomto,SCREEN_WIDTH,32;diffuse,Color.Black;y,SCREEN_BOTTOM;x,SCREEN_CENTER_X);
	};
};
return t;