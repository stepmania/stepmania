local Color = color(Var "Color1");
local children = {
	LoadActor(Var "File1") .. {
		OnCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;diffuse,Color);
		GainFocusCommand=cmd(play);
		LoseFocusCommand=cmd(pause);
	};
};

return Def.ActorFrame { children = children };
