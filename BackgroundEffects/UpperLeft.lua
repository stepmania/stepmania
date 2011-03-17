-- Upper left corner of the screen.
local cColor1 = color(Var "Color1");
local t = Def.ActorFrame {
	LoadActor(Var "File1") .. {
		OnCommand=cmd(diffuse,cColor1);
		GainFocusCommand=cmd(play);
		LoseFocusCommand=cmd(pause);
	};
};

return t;

