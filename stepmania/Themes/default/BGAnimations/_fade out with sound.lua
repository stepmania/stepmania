local t = Def.ActorFrame {
	children = {
		LoadActor( THEME:GetPathS("", "_swoosh normal") ) .. {
			StartTransitioningCommand=cmd(play);
		};
		LoadActor( THEME:GetPathB("", "_fade out normal") );
	};
};
return t;
