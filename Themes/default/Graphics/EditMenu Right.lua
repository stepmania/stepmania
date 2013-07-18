local t = Def.ActorFrame{
	LoadActor("EditMenu Left")..{
		BeginCommand=cmd(zoomx,-1);
	};
};

return t;