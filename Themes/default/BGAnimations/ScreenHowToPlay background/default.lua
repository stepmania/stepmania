return LoadActor("bg.png") ..{
	InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;zoomtowidth,SCREEN_WIDTH;zoomtoheight,SCREEN_HEIGHT);
	OnCommand=cmd(texcoordvelocity,0,-1;customtexturerect,0,0,SCREEN_WIDTH/self:GetWidth(),SCREEN_HEIGHT/self:GetHeight();diffuse,color("0.9,0.9,0.9,1"));
};