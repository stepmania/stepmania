
local t = Def.ActorFrame{}


t[#t+1] = LoadActor("UpLeft_blend")..{
	InitCommand=function(self)
		self:diffuseshift();
		self:effectcolor1(color("#9b376dFF"));
		self:effectcolor2(color("#9b376dFF"));
		self:fadetop(0.5);
	end;
}

t[#t+1] = LoadActor("UpLeft_blend")..{
	InitCommand=function(self)
		self:blend(Blend.Add);
		self:diffuseshift();
		self:effectcolor1(color("#cc5176FF"));
		self:effectcolor2(color("#cc517633"));
		self:effectclock("bgm");
		self:effecttiming(1, 0, 0, 0);
	end;
}

t[#t+1] = LoadActor("UpLeft_fill")..{
	InitCommand=function(self)
		self:blend(Blend.Add);
		self:diffuseshift();
		self:effectcolor1(color("#cc5176FF"));
		self:effectcolor2(color("#cc5176FF"));
	end;
}


t[#t+1] = LoadActor("UpLeft border");


return t