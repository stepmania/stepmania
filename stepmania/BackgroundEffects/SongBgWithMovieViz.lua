local Color1 = color(Var "Color1");
local Color2 = color(Var "Color2");

local children = {
	Def.Sprite {
		OnCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;ZoomToWidth,SCREEN_WIDTH;ZoomToHeight,SCREEN_HEIGHT;diffuse,Color1);
	};

	LoadActor(Var "File1") .. {
		OnCommand=cmd(blend,"BlendMode_Add";x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;ZoomToWidth,SCREEN_WIDTH;ZoomToHeight,SCREEN_HEIGHT;diffuse,Color2);
		GainFocusCommand=cmd(play);
		LoseFocusCommand=cmd(pause);
	};
};

return Def.ActorFrame { children = children };
