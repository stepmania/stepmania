return Def.ActorFrame {
	Def.Quad {
		InitCommand=function(self)
			self:horizalign(left);
			self:vertalign(top);
			self:y(SCREEN_TOP + 8);
		end;
		OnCommand=function(self)
			self:diffuse(Color.Black);
			self:diffusealpha(0.5);
			self:zoomto(256, 84);
			self:faderight(1);
		end;
	};
	Def.Quad {
		InitCommand=function(self)
			self:horizalign(right);
			self:vertalign(top);
			self:x(SCREEN_RIGHT);
			self:y(SCREEN_TOP+8);
		end;
		OnCommand=function(self)
			self:diffuse(Color.Black);
			self:diffusealpha(0.5);
			self:zoomto(256, 46);
			self:fadeleft(1);
		end;
	};
};