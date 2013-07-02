return Def.Quad{
	InitCommand=function(self)
		self:Center();
		self:FullScreen();
		self:diffuse(HSV(192,1,0.8));
		self:diffusebottomedge(HSV(192,0.625,0.25));
	end;
};