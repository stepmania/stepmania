return LoadActor("_CenterHit")..{
	InitCommand=function(self)
		self:x(2);
		self:y(2);
	end;
}