local Color = color(Var "Color1");

local t = Def.ActorFrame {
	LoadActor(Var "File1") .. {
		OnCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;scale_or_crop_background;diffuse,Color1;position,0;effectclock,"music");
		GainFocusCommand=cmd(play);
		LoseFocusCommand=cmd(pause);
	};
};

return t;

