local t = Def.ActorFrame{}


t[#t+1] = LoadActor("Mine_Base");

t[#t+1] = LoadActor("Mine_Fill")..{
	InitCommand=function(self)
		self:diffuseshift();
		self:effectcolor1(color("#FFFFFFFF"));
		self:effectcolor2(color("#FFFFFF22"));
		self:effectclock("bgm");
		self:effectperiod(2);
	end;
}
t[#t+1] = LoadActor("Mine_Fill")..{
	InitCommand=function(self)
		self:blend(Blend.Add);
		self:diffuseshift();
		self:effectcolor1(color("#FFFFFFFF"));
		self:effectcolor2(color("#FFFFFF22"));
		self:effectclock("bgm");
		self:effectperiod(2);
	end;
}


t[#t+1] = LoadActor("Mine_Border")..{
	InitCommand=function(self)
		self:spin();
		self:effectmagnitude(0, 0, 36);
	end;
}

t[#t+1] = LoadActor("Mine_Overlay");



t[#t+1] = LoadActor("Mine_Light")..{
	InitCommand=function(self)
		self:blend(Blend.Add);
		self:diffuseshift();
		self:effectcolor1(color("#FFFFFF55"));
		self:effectcolor2(color("#FFFFFF00"));
		self:effectclock("bgm");
		self:zoom(1.15);
		self:effectperiod(2);
	end;
}

return t