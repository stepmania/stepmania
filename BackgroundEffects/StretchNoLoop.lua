local Color1 = color(Var "Color1");

local t = Def.ActorFrame {
	LoadActor(Var "File1") .. {
		OnCommand=function(self)
			self:xy(SCREEN_CENTER_X,SCREEN_CENTER_Y)
			self:scale_or_crop_background()
			self:diffuse(Color1)
			-- The playback rate in the simfile is used to set the update rate
			-- on the ActorFrame, but effectclock("music") causes the sprite to
			-- ignore the passed in delta time and use the music time instead.
			-- So this passes the update rate in to the texture. -Kyz
			if self.GetTexture then
				self:GetTexture():rate(self:GetParent():GetUpdateRate())
			end
			if self.loop then
				self:loop(false)
				-- make videos start at beginning to prevent sticking on last frame
				self:position(0)
			end
			self:effectclock("music")
		end;
		GainFocusCommand=cmd(play);
		LoseFocusCommand=cmd(pause);
	};
};

return t;
