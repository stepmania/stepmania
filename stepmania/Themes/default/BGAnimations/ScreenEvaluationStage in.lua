local t = Def.ActorFrame {
	LoadActor( THEME:GetPathS("", "_swoosh normal") ) .. {
		StartTransitioningCommand=cmd(play);
	};
	Def.Actor { OnCommand=cmd(sleep,2); };
	LoadSongBackground() .. {
		Condition=not STATSMAN:GetCurStageStats():AllFailed();
		OnCommand=cmd(diffusealpha,1;linear,0.4;diffusealpha,0);
	};
	LoadActor( "_black" ) .. {
		Condition=STATSMAN:GetCurStageStats():AllFailed();
		InitCommand=cmd(stretchto,SCREEN_LEFT,SCREEN_TOP,SCREEN_RIGHT,SCREEN_BOTTOM);
		OnCommand=cmd(diffusealpha,1;linear,0.2;diffusealpha,0);
	};
};
return t;
