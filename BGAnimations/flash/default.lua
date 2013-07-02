local flashColor = color(Var "Color1")
return Def.Quad {
	InitCommand=function(self)
		self:x(SCREEN_CENTER_X);
		self:y(SCREEN_CENTER_Y);
		self:scaletoclipped(SCREEN_WIDTH * 2, SCREEN_HEIGHT * 2);
		self:diffuse(flashColor);
	end;
	GainFocusCommand=function(self)
		self:finishtweening();
		self:diffusealpha(1);
		self:accelerate(0.6);
		self:diffusealpha(0);
	end;
};