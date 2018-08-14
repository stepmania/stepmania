local t = Def.ActorFrame {};

t[#t+1] = Def.ActorFrame {
	Def.Quad {
		InitCommand=cmd(vertalign,bottom;zoomto,SCREEN_WIDTH,5;addy,-50;diffuse,Color("Black");fadetop,1;diffusealpha,0.3);
	};
};

t[#t+1] = Def.ActorFrame {
	Def.Quad {
		InitCommand=cmd(vertalign,bottom;zoomto,SCREEN_WIDTH,50);
		OnCommand=function(self)
		self:diffuse(ColorMidTone(ScreenColor(SCREENMAN:GetTopScreen():GetName())))
		self:diffusetopedge(ColorDarkTone(ScreenColor(SCREENMAN:GetTopScreen():GetName()))):diffusealpha(1)
		end;
	};
};

return t;
