local t = Def.ActorFrame {
	LoadActor("_warning") .. {
		InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;
			vertalign,top;
			wag;effectmagnitude,0,0,10;effectperiod,2;
		);
		OnCommand=cmd(diffusealpha,0);
		ShowDangerAllMessageCommand=cmd(stoptweening;accelerate,0.3;diffusealpha,1);
		HideDangerAllMessageCommand=cmd(stoptweening;accelerate,0.3;diffusealpha,0);
	};
};

return t;
