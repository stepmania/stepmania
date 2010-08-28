-- 4 repeats of the same file using File 1
local cColor1 = color(Var "Color1");
local a = LoadActor(Var "File1") .. {
	OnCommand=cmd(scaletoclipped,SCREEN_WIDTH,SCREEN_HEIGHT;diffuse,cColor1);
	GainFocusCommand=cmd(play);
	LoseFocusCommand=cmd(pause);
};
local t = Def.ActorFrame {
	a .. { OnCommand=cmd(x,scale(1,0,4,SCREEN_LEFT,SCREEN_RIGHT);y,scale(1,0,4,SCREEN_TOP,SCREEN_BOTTOM)); };
	a .. { OnCommand=cmd(x,scale(3,0,4,SCREEN_LEFT,SCREEN_RIGHT);y,scale(1,0,4,SCREEN_TOP,SCREEN_BOTTOM)); };
	a .. { OnCommand=cmd(x,scale(1,0,4,SCREEN_LEFT,SCREEN_RIGHT);y,scale(3,0,4,SCREEN_TOP,SCREEN_BOTTOM)); };
	a .. { OnCommand=cmd(x,scale(3,0,4,SCREEN_LEFT,SCREEN_RIGHT);y,scale(3,0,4,SCREEN_TOP,SCREEN_BOTTOM)); };
};

return t;
