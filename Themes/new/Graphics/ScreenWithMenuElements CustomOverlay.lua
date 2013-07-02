return Def.ActorFrame {
	InitCommand=function(self)
		self:x(SCREEN_CENTER_X);
		self:y(SCREEN_BOTTOM);
	end;
	Def.Quad {
		InitCommand=function(
			self:vertalign(bottom);
			self:zoomto(SCREEN_WIDTH, 32);
			self:diffuse(Color.Black);
		end;
	};
};