local t = Def.ActorFrame {};
--
t[#t+1] = Def.ActorFrame {
	InitCommand=function(self)
		self:Center();
	end;
	--
	Def.Quad {
		InitCommand=function(self)
			self:zoomto(SCREEN_WIDTH, SCREEN_HEIGHT);
			self:diffuse(Color.Black);
			self:diffusealpha(1);
		end;
		StartTransitioningCommand=function(self)
			self:smooth(0.2);
			self:diffusealpha(0);
		end;
	};
};
--
return t