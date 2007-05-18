local t = Def.ActorFrame {
	LoadActor( "_music scroll background" ) .. {
		InitCommand = cmd(scale_or_crop_background);
	};
	LoadFont( "ScreenMusicScroll", "thanks" ) .. {
		Text = "Thank you for playing!";
		InitCommand = cmd(align,0,1;x,SCREEN_LEFT+20;y,SCREEN_BOTTOM-40;zoom,.8)
	};
}
return t;
