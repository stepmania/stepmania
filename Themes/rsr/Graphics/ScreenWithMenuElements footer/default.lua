local t = Def.ActorFrame {};
t[#t+1] = Def.ActorFrame {
	InitCommand=function(self)
		self:y(-64);
	end;
	Def.Quad {
		InitCommand=function(self)
			self:vertalign(top);
			self:zoomto(SCREEN_WIDTH - 32, 2);
		end;
		OnCommand=function(self)
			self:diffusealpha(0.5);
		end;
	};
};
return t