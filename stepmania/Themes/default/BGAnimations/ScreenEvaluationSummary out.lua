return Def.ActorFrame {
	LoadActor( THEME:GetPathS("", "_swoosh normal") ) .. {
		StartTransitioningCommand=cmd(play);
	};
        LoadActor( "_moveon" ) .. {
		InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y);
		OnCommand=cmd(diffusealpha,0;sleep,1;linear,0.3;diffusealpha,1;sleep,.5);
	};
};

