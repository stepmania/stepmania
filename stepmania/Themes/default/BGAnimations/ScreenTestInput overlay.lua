return Def.ActorFrame {
	--Def.ControllerStateDisplay {
	--	InitCommand=cmd(LoadGameController,
	--};
	Def.DeviceList {
		Font="Common normal";
		InitCommand=cmd(x,SCREEN_LEFT+20;y,SCREEN_TOP+80;zoom,1.4;horizalign,left);
	};

	Def.InputList {
		Font="Common normal";
		InitCommand=cmd(x,SCREEN_CENTER_X-250;y,SCREEN_CENTER_Y;zoom,1.4;horizalign,left;vertspacing,8);
	};
};
