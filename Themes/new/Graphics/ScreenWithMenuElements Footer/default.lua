return Def.ActorFrame {
	-- Default BG
	Def.Quad {
		InitCommand=function(self)
			self:vertalign(bottom);
			self:zoomto(SCREEN_WIDTH, 60);
			self:diffuse(Color.Black);
			self:diffusealpha(0.65);
		end;
	};
	-- Highlight
	Def.Quad {
		InitCommand=function(self)
			self:vertalign(bottom);
			self:zoomto(SCREEN_WIDTH, 1);
			self:diffuse(Color.Blue);
			self:fadeleft(1);
			self:y(-58);
		end;
	};
};