local t = Def.ActorFrame{
	Def.Quad{
		InitCommand=cmd(zoomto,SCREEN_WIDTH,120;diffuse,color("#EEF4FFCC"));
	};
	LoadFont("_frutiger roman 24px")..{
		Name="Header";
		Text=THEME:GetString("ScreenGuideMain","Header");
		InitCommand=cmd(x,-312;y,-48;halign,0;diffuse,color("#111111FF");diffusetopedge,color("#11111188");shadowlength,-1;shadowcolor,color("#FFFFFF44"));
	};
	LoadFont("_frutiger roman 24px")..{
		Name="Header";
		Text=THEME:GetString("ScreenGuideMain","Explanation");
		InitCommand=cmd(x,-312;y,-28;align,0,0;diffuse,color("#111111FF");zoom,0.65);
	};
};

return t;