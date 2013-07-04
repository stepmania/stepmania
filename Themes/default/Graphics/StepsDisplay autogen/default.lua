local t = Def.ActorFrame{};

t[#t+1] = LoadActor("_badge") .. {
	InitCommand=function(self)
		self:shadowlength(1);
		self:diffuse(Color.Green);
		self:pulse();
		self:effectmagnitude(0.875, 1, 1);
		self:effecttiming(0, 0, 1, 0);
		self:effectclock('beatnooffset');
		self:effectperiod(2);
	end;
};
t[#t+1] = Def.Quad {
	InitCommand=function(self)
		self:zoomto(40, 20);
		self:diffuse(Color.Black);
		self:diffusealpha(0.5);
		self:fadeleft(0.25);
		self:faderight(0.25);
	end;
};
t[#t+1] = LoadFont("Common","Normal") .. {
	Text="AG";
	InitCommand=function(self)
		self:shadowlength(1);
		self:zoom(0.875);
	end;
};

return t;