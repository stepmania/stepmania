return Def.ActorFrame {
	LoadActor( THEME:GetPathS("", "_swoosh normal") ) .. {
		--StartTransitioningCommand=cmd(play);
	};
	LoadActor("_moveon") .. {
		OnCommand=cmd(hibernate,.1;x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;zoom,.4;zoomy,0;diffusealpha,0;linear,0.35;diffusealpha,1;zoomy,.8;zoom,.8);
	};
};
