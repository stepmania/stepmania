return Def.ActorFrame {
	LoadActor( THEME:GetPathS("", "_prompt") ) .. {
		StartTransitioningCommand=cmd(play);
	};
	Def.Actor { OnCommand=cmd(sleep,0.3) };
};
