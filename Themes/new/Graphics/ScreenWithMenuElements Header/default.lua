return Def.ActorFrame {
	-- Default BG
	Def.Quad {
		InitCommand=function(self)
			self:vertalign(top);
			self:zoomto(SCREEN_WIDTH, 48);
			self:diffuse(Color.Black);
			self:diffusealpha(0.65);
		end;
	};
	-- Highlight
	Def.Quad {
		InitCommand=function(self)
			self:vertalign(top);
			self:zoomto(SCREEN_WIDTH, 1);
			self:diffuse(Color.Blue);
			self:faderight(1);
			self:y(46);
		end;
	};
};