local t = Def.ActorFrame {};

t[#t+1] = Def.Quad {
	InitCommand=cmd(vertalign,bottom;zoomto,SCREEN_WIDTH+1,34;diffuse,Color.Black);
};

return t;