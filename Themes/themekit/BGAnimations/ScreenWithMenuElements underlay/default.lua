local t = Def.ActorFrame {
	InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y);
};
--
t[#t+1] = LoadActor("grid") .. {
	InitCommand=cmd(zoomto,SCREEN_WIDTH,SCREEN_HEIGHT;customtexturerect,0,0,SCREEN_WIDTH/16,SCREEN_HEIGHT/16);
	OnCommand=cmd(diffuse,color("0,0,0,0.125"));
};
t[#t+1] = LoadActor("grid") .. {
	InitCommand=cmd(zoomto,SCREEN_WIDTH,SCREEN_HEIGHT;customtexturerect,0,0,SCREEN_WIDTH/32,SCREEN_HEIGHT/32);
	OnCommand=cmd(diffuse,color("0,0,0,0.25"));
};
--
return t