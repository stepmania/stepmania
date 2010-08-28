-- Align left when cropping to 4:3.
local cColor1 = color(Var "Color1");
local t = Def.ActorFrame {
	LoadActor(Var "File1") .. {
		OnCommand=cmd(horizalign,left;scaletoclipped,SCREEN_WIDTH,SCREEN_HEIGHT;diffuse,cColor1);
		GainFocusCommand=cmd(play);
		LoseFocusCommand=cmd(pause);
	};
};

return t;
