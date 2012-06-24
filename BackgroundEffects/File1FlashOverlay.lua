local Color1 = color(Var "Color1");
local Color2 = color(Var "Color2");

local t = Def.ActorFrame {};

t[#t+1] = LoadActor(Var "File1") .. {
	OnCommand=cmd(diffuse,Color1;blend,"BlendMode_Add";x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;scale_or_crop_background;linear,1;diffusealpha,0);
	GainFocusCommand=cmd(play);
	LoseFocusCommand=cmd(pause);
};

return t;
