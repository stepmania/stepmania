return LoadActor( "caution" ) .. {
	InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;);
	OffCommand=cmd(diffusealpha,1;linear,.3;diffusealpha,0);
}