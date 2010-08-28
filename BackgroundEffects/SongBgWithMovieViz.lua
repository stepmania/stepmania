-- A visualization overplayed on the songs background.
local cColor1 = color(Var "Color1");
local cColor2 = color(Var "Color2");
local t = Def.ActorFrame {
	Def.Sprite {
		InitCommand=cmd(LoadFromCurrentSongBackground);
		OnCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;scaletoclipped,SCREEN_WIDTH,SCREEN_HEIGHT;diffuse,cColor1);
	};

	LoadActor(Var "File1") .. {
		OnCommand=cmd(blend,"BlendMode_Add";x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;scaletoclipped,SCREEN_WIDTH,SCREEN_HEIGHT;diffuse,cColor2);
		GainFocusCommand=cmd(play);
		LoseFocusCommand=cmd(pause);
	};
};

return t;
