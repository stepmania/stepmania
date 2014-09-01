local t = Def.ActorFrame {};

t[#t+1] = Def.Quad {
	InitCommand=cmd(vertalign,bottom;zoomto,SCREEN_WIDTH,34;diffuse,color("#161616"));
};

t[#t+1] = LoadActor(THEME:GetPathG("ScreenWithMenuElements","header/Header")) .. {
	InitCommand=cmd(y,-48;vertalign,bottom;zoomtowidth,SCREEN_WIDTH);
	OnCommand=cmd(zoomy,-1;diffuse,color("#ffd400"));
};

return t;
