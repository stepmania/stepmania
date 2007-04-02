return LoadActor( "bg.png" ) .. {
	InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y);
	OnCommand=cmd(scale_or_crop_background);
}
