local Color1 = color(Var "Color1");
local Color2 = color(Var "Color2");

local a1 = LoadActor(Var "File1") .. {
	OnCommand=function(self)
		self:cropto(SCREEN_WIDTH / 2);
		self:zoomtoheight(SCREEN_HEIGHT / 2);
		self:diffuse(Color1);
		self:effectclock("music");
	end;
	GainFocusCommand=function(self)
		self:play();
	end;
	LoseFocusCommand=function(self)
		self:pause();
	end;
};
local a2 = LoadActor(Var "File2") .. {
	OnCommand=function(self)
		self:cropto(SCREEN_WIDTH / 2);
		self:zoomtoheight(SCREEN_HEIGHT / 2);
		self:diffuse(Color2);
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
	a1 .. {
		OnCommand = function(self)
			self:x(scale(1, 0, 4, SCREEN_LEFT, SCREEN_RIGHT));
			self:y(scale(1, 0, 4, SCREEN_TOP, SCREEN_BOTTOM));
		end;
	};
	a2 .. {
		OnCommand = function(self)
			self:x(scale(3, 0, 4, SCREEN_LEFT, SCREEN_RIGHT));
			self:y(scale(1, 0, 4, SCREEN_TOP, SCREEN_BOTTOM));
		end;
	};
	a2 .. {
		OnCommand = function(self)
			self:x(scale(1, 0, 4, SCREEN_LEFT, SCREEN_RIGHT));
			self:y(scale(3, 0, 4, SCREEN_TOP, SCREEN_BOTTOM));
		end;
	};
	a1 .. {
		OnCommand = function(self)
			self:x(scale(3, 0, 4, SCREEN_LEFT, SCREEN_RIGHT));
			self:y(scale(3, 0, 4, SCREEN_TOP, SCREEN_BOTTOM));
		end;
	};
};

return t;

