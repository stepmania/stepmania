return Def.ActorFrame {
	Def.Quad {
		InitCommand=function(self)
			self:zoomto(10, 10);
			self:y(-5);
			self:skewx(1);
		end;
	};
	Def.Quad {
		InitCommand=function(self)
			self:zoomto(10, 10);
			self:y(5);
			self:skewx(-1);
		end;
	};
}