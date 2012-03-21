--- Stretch a file with green diffusion.
local Color1 = color("0,1,0,1");

local t = Def.ActorFrame {
	LoadActor(Var "File1") .. {
		OnCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;scale_or_crop_background;diffuse,Color1;effectclock,"music");
		GainFocusCommand=cmd(play);
		LoseFocusCommand=cmd(pause);
	};
};

return t;
