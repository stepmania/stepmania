return Def.ActorFrame {
	InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y);
	LoadFont("Common Normal") .. {
		Text=ScreenString("GAME OVER");
		InitCommand=cmd(y,-4;shadowlength,1;diffuse,Color("Red"));
	};
	LoadFont("Common Normal") .. {
		Text=ScreenString("Play again soon!");
		InitCommand=cmd(y,16;shadowlength,1;zoom,0.5;);
	};
};