-- Alternating files being played back at once
local t = Def.ActorFrame {
    LoadActor(Var "File1") .. { 
		OnCommand = function(self)
			self:x(scale(1, 0, 4, SCREEN_LEFT, SCREEN_RIGHT));
			self:y(scale(1, 0, 4, SCREEN_TOP, SCREEN_BOTTOM));
			self:cropto(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
			self:rate(0.25);
		end;
	LoadActor(Var "File2") .. { 
	OnCommand = function(self)
			self:x(scale(3, 0, 4, SCREEN_LEFT, SCREEN_RIGHT));
			self:y(scale(1, 0, 4, SCREEN_TOP, SCREEN_BOTTOM));
			self:cropto(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
			self:rate(0.25);
		end;
	};
    LoadActor(Var "File2") .. { 
	OnCommand = function(self)
			self:x(scale(1, 0, 4, SCREEN_LEFT, SCREEN_RIGHT));
			self:y(scale(3, 0, 4, SCREEN_TOP, SCREEN_BOTTOM));
			self:cropto(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
			self:rate(0.25);
		end;
	};
    LoadActor(Var "File1") .. { 
	OnCommand = function(self)
			self:x(scale(3, 0, 4, SCREEN_LEFT, SCREEN_RIGHT));
			self:y(scale(3, 0, 4, SCREEN_TOP, SCREEN_BOTTOM));
			self:cropto(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
			self:rate(0.25);
		end;
	};
};

return t