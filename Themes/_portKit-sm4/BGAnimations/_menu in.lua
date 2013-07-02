return Def.ActorFrame {
	LoadActor( THEME:GetPathS("", "_swoosh normal") ) .. {

	};
	LoadActor("_moveon") .. {
		OnCommand=function(self)
			self:x(SCREEN_CENTER_X);
			self:y(SCREEN_CENTER_Y);
			self:diffusealpha(1);
			self:linear(0.2);
			self:diffusealpha(0);
			self:zoomx(1);
			self:zoomy(0);
		end;
	};
};
