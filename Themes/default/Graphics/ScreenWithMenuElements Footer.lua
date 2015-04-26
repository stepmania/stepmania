local t = Def.ActorFrame {};

t[#t+1] = Def.Quad {
	InitCommand=cmd(vertalign,bottom;zoomto,SCREEN_WIDTH,32;diffuse,color("#161616"));
};

return t;
