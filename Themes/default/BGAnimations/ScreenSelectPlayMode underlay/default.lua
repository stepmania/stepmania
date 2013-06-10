return LoadActor( "frame" ) .. {
	InitCommand=cmd(x,SCREEN_CENTER_X-150;y,SCREEN_CENTER_Y;);
	OnCommand=cmd(zoomy,1.6;diffusealpha,0;linear,.18;zoomy,1;diffusealpha,1);
	OffCommand=cmd(zoomy,1;diffusealpha,1;linear,.18;zoomy,1.6;diffusealpha,0);
};
