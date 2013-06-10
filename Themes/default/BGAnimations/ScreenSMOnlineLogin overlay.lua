local FontName = "_venacti Bold 15px";
local OffCommand = cmd(accelerate,0.3;addx,SCREEN_WIDTH);

local t = Def.ActorFrame {
	LoadFont( FontName ) .. {
		Text = ScreenString("Depending on the server you're connected to, you may have to register for an account on their website.");
		InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y+70;shadowlength,1;wrapwidthpixels,SCREEN_WIDTH*0.75);
		OnCommand=cmd(cropright,1;linear,.5;cropright,0);
		OffCommand=OffCommand;
	};
}
return t
