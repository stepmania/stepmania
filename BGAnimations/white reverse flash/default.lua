return Def.Quad {    
    InitCommand=function(self)
		self:x(SCREEN_CENTER_X);
		self:y(SCREEN_CENTER_Y);
		self:scaletoclipped(SCREEN_WIDTH * 2, SCREEN_HEIGHT * 2);
	end;
    GainFocusCommand=function(self)
		self:finishtweening();
		self:diffusealpha(0);
		self:accelerate(0.6);
		self:diffusealpha(1);
	end;
};