-- Stretch a file with blue diffusion.
local cColor1 = color("0,0,1,1");
local t = Def.ActorFrame {
	LoadActor(Var "File1") .. {
		OnCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;scaletoclipped,SCREEN_WIDTH,SCREEN_HEIGHT;diffuse,cColor1);
		GainFocusCommand=cmd(play);
		LoseFocusCommand=cmd(pause);
	};
};

return t;
