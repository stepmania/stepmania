local t = Def.ActorFrame {
	LoadActor( THEME:GetPathS("", "_failed") ) .. {
		StartTransitioningCommand=cmd(play);
	};
	Def.Actor { OnCommand=cmd(sleep,5); };
	LoadActor("../_black.png") .. {
		InitCommand=cmd(stretchto,SCREEN_LEFT,SCREEN_TOP,SCREEN_RIGHT,SCREEN_BOTTOM);
		OnCommand=cmd(diffusealpha,0;sleep,0.5;linear,0.5;diffusealpha,1);
	};

	-- Failed with ghosts
	LoadActor("failed") .. {
		OnCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;diffusealpha,0;zoom,4;sleep,0.5;decelerate,0.6;diffusealpha,0.4;zoom,0.6;accelerate,0.4;zoom,1;decelerate,0.2;zoom,1.1;linear,0.1;zoom,1.0;sleep,2);
	};

	LoadActor("failed") .. {
		OnCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;diffusealpha,0;zoom,4;sleep,0.6;decelerate,0.6;diffusealpha,0.4;zoom,0.6;accelerate,0.4;zoom,1;decelerate,0.2;zoom,1.1;linear,0.1;zoom,1.0;sleep,2);
	};

	LoadActor("failed") .. {
		OnCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;diffusealpha,0;zoom,4;sleep,0.7;decelerate,0.6;diffusealpha,0.4;zoom,0.6;accelerate,0.4;zoom,1;decelerate,0.2;zoom,1.1;linear,0.1;zoom,1.0;sleep,2);
	};

	LoadActor("failed") .. {
		OnCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;diffusealpha,0;zoom,4;sleep,0.8;decelerate,0.6;diffusealpha,0.4;zoom,0.6;accelerate,0.4;zoom,1;decelerate,0.2;zoom,1.1;linear,0.1;zoom,1.0;sleep,2);
	};
};

return t;
