local t = Def.ActorFrame {};

t[#t+1] = Def.Quad {
	InitCommand=function(self)
		self:vertalign(bottom);
		self:zoomto(SCREEN_WIDTH + 1, 34);
		self:diffuse(Color.Black);
	end;
};

return t;