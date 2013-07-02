local Color1 = color(Var "Color1");

local a = LoadActor(Var "File1") .. {
	OnCommand=function(self)
		self:cropto(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
		self:diffuse(Color1);
		self:zoomx(self:GetZoomX() * -1);
		self:zoomy(self:GetZoomY() * -1);
		self:effectclock("music");
	end;
	GainFocusCommand=function(self)
		self:play();
	end;
	LoseFocusCommand=function(self)
		self:pause();
	end;
};

local t = Def.ActorFrame {
	a .. {
		OnCommand = function(self)
			self:x(scale(1, 0, 4, SCREEN_LEFT, SCREEN_RIGHT));
			self:y(scale(1, 0, 4, SCREEN_TOP, SCREEN_BOTTOM));
		end;
	};
	a .. {
		OnCommand = function(self)
			self:x(scale(3, 0, 4, SCREEN_LEFT, SCREEN_RIGHT));
			self:y(scale(1, 0, 4, SCREEN_TOP, SCREEN_BOTTOM));
		end;
	};
	a .. {
		OnCommand = function(self)
			self:x(scale(1, 0, 4, SCREEN_LEFT, SCREEN_RIGHT));
			self:y(scale(3, 0, 4, SCREEN_TOP, SCREEN_BOTTOM));
		end;
	};
	a .. {
		OnCommand = function(self)
			self:x(scale(3, 0, 4, SCREEN_LEFT, SCREEN_RIGHT));
			self:y(scale(3, 0, 4, SCREEN_TOP, SCREEN_BOTTOM));
		end;
	};
};

return t;
