return Def.ActorFrame{
	InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y),
	Def.Quad{
		InitCommand=cmd(zoomto,SCREEN_WIDTH,SCREEN_HEIGHT;diffuse,Color.Black;diffusealpha,0),
		TweenOnCommand=cmd(stoptweening;diffusealpha,1;linear,0.5;diffusealpha,0.8),
		TweenOffCommand=cmd(stoptweening;linear,0.5;diffusealpha,0),
	},
	Def.ActorFrame{
		Def.BitmapText{
			Font="Common Normal",
			Text=ScreenString("NoSetListPrompt"),
			InitCommand=cmd(y,10;wrapwidthpixels,SCREEN_WIDTH-48;diffusealpha,0),
			TweenOnCommand=cmd(stoptweening;diffusealpha,0;sleep,0.5125;linear,0.125;diffusealpha,1),
			TweenOffCommand=cmd(stoptweening;linear,0.5;diffusealpha,0),
		},
	},
}
