return LoadActor(THEME:GetPathG("MusicWheelItem", "ModeItem")) .. {
	OnCommand=function(self)
		self:diffuse(ScreenColor(SCREENMAN:GetTopScreen():GetName()));
	end;
};