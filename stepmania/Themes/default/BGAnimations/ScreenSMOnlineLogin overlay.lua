local OffCommand = cmd(accelerate,0.3;addx,SCREEN_WIDTH)
local t = Def.ActorFrame {
	LoadFont( "blaster" ) .. {
		Text = "Depending on the server you're connected to,";
		InitCommand = cmd(zoom,.75;x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y+90);
		OnCommand = cmd(cropright,1;linear,.5;cropright,0);
		OffCommand = OffCommand;
	};
	LoadFont( "blaster" ) .. {
		Text = "you may have to register for an account";
		InitCommand = cmd(zoom,.75;x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y+107);
		OnCommand = cmd(cropright,1;sleep,.5;linear,.38;cropright,0);
		OffCommand = OffCommand;
	};
	LoadFont( "blaster" ) .. {
		Text = "on their website.";
		InitCommand = cmd(zoom,.75;x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y+124);
		OnCommand(cropright,1;sleep,.9;linear,.25;cropright,0);
		OffCommand = OffCommand;
	};
}
return t
