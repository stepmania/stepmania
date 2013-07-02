return Def.ActorFrame {
	LoadActor( THEME:GetPathS("", "_swoosh normal") ) .. {

	};
	LoadActor("_moveon") .. {
		OnCommand=function(self)
			self:hibernate(0.1);
			self:x(SCREEN_CENTER_X);
			self:y(SCREEN_CENTER_Y);
			self:zoomx(1);
			self:zoomy(0);
			self:diffusealpha(0);
			self:linear(0.35);
			self:diffusealpha(1);
			self:zoom(1);
		end;
	};
};
