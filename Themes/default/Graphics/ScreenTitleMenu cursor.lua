local t = Def.ActorFrame {
	Def.Quad {
		InitCommand=function(self)
			self:zoomto(32, 32);
		end;
		OnCommand=function(self)
			self:spin();
		end;
	};
};

return t