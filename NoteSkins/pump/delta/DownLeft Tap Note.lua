local t = Def.ActorFrame{}


t[#t+1] = LoadActor("DownLeft_blend")..{
	InitCommand=fnction(self)
		self:diffuseshift();
		self:effectcolor1(color("#376f9b66"));
		self:effectcolor2(color("#376f9b66"));
	end;
}

t[#t+1] = LoadActor("DownLeft_blend")..{
	InitCommand=function(self)
		self:blend(Blend.Add);
		self:diffuseshift();
		self:effectcolor1(color("#5899ccFF"));
		self:effectcolor2(color("#5899cc66"));
		self:effectclock("bgm");
		self:effecttiming(1, 0, 0, 0);
	end;
}

t[#t+1] = LoadActor("DownLeft_blend")..{
	InitCommand=function(self)
		self:diffuseshift();
		self:effectcolor1(color("#5899ccFF"));
		self:effectcolor2(color("#5899ccFF"));
		self:fadetop(1);
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