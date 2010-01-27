return Def.Quad{
	OnCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;zoomtowidth,SCREEN_WIDTH;zoomtoheight,SCREEN_HEIGHT);
	GainFocusCommand=cmd(diffusealpha,1.0;accelerate,0.6;diffusealpha,0);
}
