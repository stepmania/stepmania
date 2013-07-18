return Def.ActorFrame {
	-- Default BG
	Def.Quad {
		InitCommand=cmd(vertalign,top;zoomto,SCREEN_WIDTH,48;diffuse,Color.Black;diffusealpha,0.65);
	};
	-- Highlight
	Def.Quad {
		InitCommand=cmd(vertalign,top;zoomto,SCREEN_WIDTH,1;diffuse,Color.Blue;faderight,1;y,46);
	};
};