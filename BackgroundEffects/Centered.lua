local Color = color(Var "Color1");
local t = Def.ActorFrame {
	LoadActor(Var "File1") .. {
		OnCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;diffuse,Color;effectclock,"music");
		GainFocusCommand=cmd(play);
		LoseFocusCommand=cmd(pause);
	};
};

return t;
