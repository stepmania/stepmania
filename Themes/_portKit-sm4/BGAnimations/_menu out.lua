return Def.ActorFrame {
	LoadActor( THEME:GetPathS("", "_swoosh normal") ) .. {
		--StartTransitioningCommand=cmd(play);
	};
	LoadActor("_moveon") .. {
		OnCommand=cmd(hibernate,0.1;x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;zoomx,1;zoomy,0;diffusealpha,0;linear,0.35;diffusealpha,1;zoom,1);
	};
};
