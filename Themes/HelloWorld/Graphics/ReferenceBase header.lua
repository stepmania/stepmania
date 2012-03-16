local t = Def.ActorFrame{
	Def.Quad{
		InitCommand=cmd(zoomto,SCREEN_WIDTH,96;diffuse,color("#EEF4FFCC"));
	};
	Def.ActorFrame{
		InitCommand=cmd(y,12);
		LoadFont("_frutiger roman 24px")..{
			Name="Header";
			Text=ScreenString("Header");
			InitCommand=cmd(x,-312;y,-34;halign,0;diffuse,color("#111111FF");diffusetopedge,color("#11111188");shadowlength,-1;shadowcolor,color("#FFFFFF44"));
		};
		Def.Quad{
			InitCommand=cmd(zoomto,SCREEN_WIDTH*0.975,2;y,-20;diffuse,color("#111111FF");diffusetopedge,color("#11111188");shadowlength,-1;shadowcolor,color("#FFFFFF44");fadeleft,0.1;faderight,0.625);
		};
		LoadFont("_frutiger roman 24px")..{
			Name="Explanation";
			Text=ScreenString("Explanation");
			InitCommand=cmd(x,-312;y,-14;align,0,0;diffuse,color("#111111FF");zoom,0.65;wrapwidthpixels,(SCREEN_WIDTH));
		};
	};
};

return t;