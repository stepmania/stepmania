return Def.ActorFrame {
	LoadActor( THEME:GetPathS("", "_swoosh normal") ) .. {
		StartTransitioningCommand=cmd(play);
	};

        Def.Actor { OnCommand=cmd(sleep,1.3); };

	LoadActor( "_moveon" ) .. {
		OnCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;zoomy,1;diffusealpha,1;linear,0.5;diffusealpha,0;zoomy,0);
	};
};
