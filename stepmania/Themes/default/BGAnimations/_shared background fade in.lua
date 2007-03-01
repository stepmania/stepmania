local children = {
	LoadActor( THEME:GetPathB("","_shared background normal") );
	LoadActor( THEME:GetPathB("","_fade in normal") ) .. {
		OnCommand=cmd(playcommand,"StartTransitioning");
	};
};

return Def.ActorFrame { children=children };
