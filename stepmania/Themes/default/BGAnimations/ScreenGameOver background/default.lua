local t = Def.ActorFrame {
	LoadActor("gameover.png") .. {
		InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;diffusealpha,0);
		OnCommand=cmd(linear,0.3;diffusealpha,0.9;sleep,5);
	};
};

return t;
