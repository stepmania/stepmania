return Def.ActorFrame{
	Def.Quad{
		Name="Horizontal";
		InitCommand=cmd(x,48;y,14;zoomto,SCREEN_CENTER_X,32;diffuse,color("1,1,1,0.175");blend,Blend.Add;vertalign,bottom;fadeleft,0.25;faderight,0.5);
	};
	Def.Quad{
		Name="Vertical";
		InitCommand=cmd(x,48;y,14;zoomto,SCREEN_CENTER_X,32;diffuse,color("1,1,1,0.05");blend,Blend.Add;vertalign,bottom;fadetop,0.85;);
	};

	-- bottom line
	Def.Quad{
		InitCommand=cmd(zoomto,SCREEN_WIDTH,2;y,16;diffuserightedge,HSV(192,1,1);shadowlengthy,1.75;shadowcolor,HSVA(204,0.625,0.5,0.5));
		OffCommand=cmd(linear,0.5;cropleft,1);
	};
};