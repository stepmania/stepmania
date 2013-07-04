return LoadFont("Combo Numbers") .. {
	InitCommand=function(self)
		self:zoom(12/54);
		self:y(-1);
		self:shadowlengthx(0);
		self:shadowlengthy(1);
		self:strokecolor(color("#00000077"));
	end;
};