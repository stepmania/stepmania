local t = Def.ActorFrame {
	Def.Quad {
		InitCommand=cmd(zoomto,32,32);
		OnCommand=cmd(spin);
	};
};

return t