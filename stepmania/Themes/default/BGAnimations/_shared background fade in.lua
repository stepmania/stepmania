local children = {
	LoadActor( "_shared background normal" );
	LoadActor( "_fade in normal" ) .. {
		OnCommand=cmd(playcommand,"StartTransitioning");
	};
};

return Def.ActorFrame { children=children };
