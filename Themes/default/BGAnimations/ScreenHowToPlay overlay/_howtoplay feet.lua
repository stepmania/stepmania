return LoadFont("Common Normal") .. {
	Text="Your feet will be used to play!";
	BeginCommand=function(self)
		self:AddAttribute(5, {Length= 4, Diffuse=Color.Red})
	end;
};
