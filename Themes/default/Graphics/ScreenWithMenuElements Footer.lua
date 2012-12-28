local t = Def.ActorFrame {};

t[#t+1] = Def.Quad {
	InitCommand=cmd(vertalign,bottom;zoomto,SCREEN_WIDTH+1,32;diffuse,Color.Black);
};

return t;