return Def.ActorFrame {
	Def.Quad {
		InitCommand=cmd(zoomto,SCREEN_WIDTH,SCREEN_HEIGHT;Center);
		OnCommand=cmd(diffuse,Color.Black;diffusealpha,0;decelerate,0.3;diffusealpha,0.7);
		OffCommand=cmd(stoptweening;decelerate,0.2;diffusealpha,0);
	};
	Def.DeviceList {
		Font="Common Italic Condensed",
		InitCommand=cmd(x,SCREEN_LEFT+20;y,SCREEN_TOP+130;zoom,0.8;halign,0;diffuse,color("#FFFFFF"));
		OffCommand=cmd(stoptweening;decelerate,0.2;diffusealpha,0);
	};

	Def.InputList {
		Font="Common Condensed",
		InitCommand=cmd(x,SCREEN_CENTER_X-250;y,SCREEN_CENTER_Y;zoom,1;halign,0;vertspacing,8;strokecolor,color("#000000"));
	};
};
