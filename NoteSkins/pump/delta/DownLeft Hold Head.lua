local t = Def.ActorFrame{}


t[#t+1] = LoadActor("DownLeft_blend")..{
	InitCommand=function(self)
		self:diffuseshift();
		self:effectcolor1(color("#376f9bFF"));
		self:effectcolor2(color("#376f9bFF"));
		self:fadetop(0.5);
	end;
}

t[#t+1] = LoadActor("DownLeft_blend")..{
	InitCommand=function(self)
		self:blend(Blend.Add);
		self:diffuseshift();
		self:effectcolor1(color("#5899ccFF"));
		self:effectcolor2(color("#5899cc33"));
		self:effectclock("bgm");
		self:effecttiming(1, 0, 0, 0);
	end;
}

t[#t+1] = LoadActor("DownLeft_fill")..{
	InitCommand=function(self)
		self:blend(Blend.Add);
		self:diffuseshift();
		self:effectcolor1(color("#5899ccFF"));
		self:effectcolor2(color("#5899ccFF"));
	end;
}


t[#t+1] = LoadActor("DownLeft border");


return t