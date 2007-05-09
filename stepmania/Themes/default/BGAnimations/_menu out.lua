return Def.ActorFrame {
	LoadActor( THEME:GetPathS("", "_swoosh normal") ) .. {
		StartTransitioningCommand=cmd(play);
	};
	LoadActor("_moveon") .. {
		OnCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;zoomy,0;diffuse,0,0,0,0;linear,0.5;diffuse,1,1,1,1;zoomy,1);
	};
};
