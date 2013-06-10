local t = Def.ActorFrame { };

t[#t+1] = LoadActor( THEME:GetPathB("","_shared background normal") );
t[#t+1] = LoadActor( THEME:GetPathB("","_fade in normal") ) .. {
	OnCommand=cmd(playcommand,"StartTransitioning");
};

return t;
