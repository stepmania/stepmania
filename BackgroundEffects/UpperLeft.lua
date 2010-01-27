-- upper left corner
local Color = color(Var "Color1");

local t = Def.ActorFrame {
	LoadActor(Var "File1") .. {
		OnCommand=cmd(diffuse,Color;effectclock,"music");
		GainFocusCommand=cmd(play);
		LoseFocusCommand=cmd(pause);
	};
};

return t;

