return Def.ActorFrame {
	Def.Quad {
		InitCommand=cmd(horizalign,left;vertalign,top;y,SCREEN_TOP+8);
		OnCommand=cmd(diffuse,Color.Black;diffusealpha,0.5;zoomto,256,84;faderight,1);
	};
	Def.Quad {
		InitCommand=cmd(horizalign,right;vertalign,top;x,SCREEN_RIGHT;y,SCREEN_TOP+8);
		OnCommand=cmd(diffuse,Color.Black;diffusealpha,0.5;zoomto,256,46;fadeleft,1);
	};
};