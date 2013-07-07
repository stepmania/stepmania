return LoadActor("_DownLeftHit")..{
	InitCommand=fnction(self)
		self:y(5);
		self:x(2);
	end;
}