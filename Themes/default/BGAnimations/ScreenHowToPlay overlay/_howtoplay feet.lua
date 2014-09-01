return LoadFont("Common Normal") .. {
	Text=ScreenString("Feet");
	BeginCommand=function(self)
		self:AddAttribute(5, {Length= 4, Diffuse=Color.Red})
	end;
};
