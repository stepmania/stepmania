-- upper left corner
local Color = color(Var "Color1");

local children = {
	LoadActor(Var "File1") .. {
		OnCommand=cmd(diffuse,Color);
		GainFocusCommand=cmd(play);
		LoseFocusCommand=cmd(pause);
	};
};

return Def.ActorFrame { children = children };

