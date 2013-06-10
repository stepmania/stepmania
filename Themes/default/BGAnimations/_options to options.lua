local t = Def.ActorFrame {
	LoadActor( THEME:GetPathS("", "_swoosh normal") ) .. {
		StartTransitioningCommand=cmd(play);
	};
	Def.Actor { OnCommand=cmd(sleep,0.5); };
};
return t;
