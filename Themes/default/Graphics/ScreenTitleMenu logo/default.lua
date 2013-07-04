local t = Def.ActorFrame{
	InitCommand=function(self)
		self:fov(70);
	end;
	LoadActor("_arrow")..{
		InitCommand=function(self)
			self:x(225);
		end;
		OnCommand=function(self)
			self:wag();
			self:effectmagnitude(0, 0, 16);
			self:effectperiod(2.5);
		end;
	};
	LoadActor("_text");
	LoadActor("_text")..{
		Name="TextGlow";
		InitCommand=function(self)
			self:blend(Blend.Add);
			self:diffusealpha(0.05);
		end;
		OnCommand=function(self)
			self:glowshift();
			self:effectperiod(2.5);
			self:effectcolor1(color("1,1,1,0.25"));
			self:effectcolor2(color("1,1,1,1"));
		end;
	};
};

return t;
