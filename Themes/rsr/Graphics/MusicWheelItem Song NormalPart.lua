return Def.ActorFrame {
	Def.Quad {
		InitCommand=function(self)
			self:x(4);
			self:zoomto(252, 46);
		end;
		OnCommand=function(self)
			self:diffuse(color("0,0,0,0.5"));
		end;
	};
};