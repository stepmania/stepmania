local Color = color(Var "Color1");

local children = {
	LoadActor(Var "File1") .. {
		OnCommand=cmd(scale_or_crop_background;diffuse,Color;position,0);
		GainFocusCommand=cmd(play);
		LoseFocusCommand=cmd(pause);
	};
};

return Def.ActorFrame { children = children };

