local Color1 = color(Var "Color1");
local stretchBG = PREFSMAN:GetPreference("StretchBackgrounds")

local t = Def.ActorFrame {
	LoadActor(Var "File1") .. {
		OnCommand=function(self)
			self:xy(SCREEN_CENTER_X,SCREEN_CENTER_Y)
			if stretchBG then self:SetSize(SCREEN_WIDTH,SCREEN_HEIGHT)
			else self:scale_or_crop_background();
			end
			self:diffuse(Color1)
			self:effectclock("music")
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
