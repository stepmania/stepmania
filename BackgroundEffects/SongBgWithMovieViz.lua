local Color1 = color(Var "Color1");
local Color2 = color(Var "Color2");
local stretchBG = PREFSMAN:GetPreference("StretchBackgrounds")

local t = Def.ActorFrame {
	Def.Sprite {
		OnCommand=function(self)
			self:LoadFromCurrentSongBackground()
			self:xy(SCREEN_CENTER_X,SCREEN_CENTER_Y)
			if stretchBG then
				self:SetSize(SCREEN_WIDTH,SCREEN_HEIGHT)
			else
				self:scale_or_crop_background();
			end
			self:diffuse(Color1)
			self:effectclock("music")
		end;
	};

	LoadActor(Var "File1") .. {
		OnCommand=function(self)
			self:blend("BlendMode_Add");
			self:x(SCREEN_CENTER_X);
			self:y(SCREEN_CENTER_Y);
			self:scale_or_crop_background();
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
};

return t;
