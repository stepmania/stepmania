
local t = Def.ActorFrame{}


t[#t+1] = LoadActor("UpLeft_blend")..{
	InitCommand=function(self)
		self:diffuseshift();
		self:effectcolor1(color("#9b376d66"));
		self:effectcolor2(color("#9b376d66"));
	end;
}

t[#t+1] = LoadActor("UpLeft_blend")..{
	InitCommand=function(self)
		self:blend(Blend.Add);
		self:diffuseshift();
		self:effectcolor1(color("#cc5176FF"));
		self:effectcolor2(color("#cc517666"));
		self:effectclock("bgm");
		self:effecttiming(1, 0, 0, 0);
	end;
}


t[#t+1] = LoadActor("UpLeft_blend")..{
	InitCommand=function(self)
		self:diffuseshift();
		self:effectcolor1(color("#cc5176FF"));
		self:effectcolor2(color("#cc5176FF"));
		self:fadetop(1);
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