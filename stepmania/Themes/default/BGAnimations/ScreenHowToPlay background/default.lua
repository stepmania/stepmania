return LoadActor("bg.png") ..{
	InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;zoomtowidth,SCREEN_WIDTH;zoomtoheight,SCREEN_HEIGHT);
	OnCommand=cmd(texcoordvelocity,0,-1;customtexturerect,0,0,5,5;diffuse,0.8,0.8,0.8,1);
};