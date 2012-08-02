return Def.ActorFrame {
	-- Default BG
	Def.Quad {
		InitCommand=cmd(vertalign,bottom;zoomto,SCREEN_WIDTH,60;diffuse,Color.Black;diffusealpha,0.65);
	};
	-- Highlight
	Def.Quad {
		InitCommand=cmd(vertalign,bottom;zoomto,SCREEN_WIDTH,1;diffuse,Color.Blue;fadeleft,1;y,-58);
	};
};