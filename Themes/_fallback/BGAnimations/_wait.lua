local time = ...
assert(time)
return Def.ActorFrame{
	Def.Actor{ OnCommand=cmd(sleep,time); };
};