local t = Def.ActorFrame {};

t[#t+1] = Def.ActorFrame {
	Def.Quad {
		InitCommand=function(self)
			self:zoomto(32,32);
		end;
	};
};

return t