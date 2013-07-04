local t = Def.ActorFrame {};
t[#t+1] = Def.Sprite {
	InitCommand=function(self)
		self:Center();
		self:diffusealpha(1);
	end;
	BeginCommand=function(self)
		self:LoadFromCurrentSongBackground();
	end;
	OnCommand=function(self)
		if PREFSMAN:GetPreference("StretchBackgrounds") then
			self:SetSize(SCREEN_WIDTH,SCREEN_HEIGHT)
		else
			self:scale_or_crop_background()
		end
		self:linear(1)
		self:diffusealpha(0)
	end;
};
return t;