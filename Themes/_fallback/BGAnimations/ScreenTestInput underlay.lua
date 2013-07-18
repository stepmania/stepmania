return Def.ActorFrame {
	--Def.ControllerStateDisplay {
	--	InitCommand=cmd(LoadGameController,
	--};
	Def.DeviceList {
		Font="Common normal";
		InitCommand=cmd(x,SCREEN_LEFT+20;y,SCREEN_TOP+80;zoom,0.8;halign,0);
	};

	Def.InputList {
		Font="Common normal";
		InitCommand=cmd(x,SCREEN_CENTER_X-250;y,SCREEN_CENTER_Y;zoom,1;halign,0;vertspacing,8);
	};
};
