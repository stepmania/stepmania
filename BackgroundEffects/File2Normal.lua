local Color1 = color(Var "Color1");
local Color2 = color(Var "Color2");
local stretchBG = PREFSMAN:GetPreference("StretchBackgrounds")

local t = Def.ActorFrame {};

t[#t+1] = LoadActor(Var "File1") .. {
	OnCommand=function(self)
		self:x(SCREEN_CENTER_X);
		self:y(SCREEN_CENTER_Y);
		self:scale_or_crop_background();
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

if Var("File2") ~= nil then
	t[#t+1] = LoadActor(Var("File2")) .. {
	OnCommand=function(self)
		self:x(SCREEN_CENTER_X);
		self:y(SCREEN_CENTER_Y);
		self:scale_or_crop_background();
		self:diffuse(Color2); -- originally Color1: assuming mistake.
		self:effectclock("music");
	end;
	GainFocusCommand=function(self)
		self:play();
	end;
	LoseFocusCommand=function(self)
		self:pause();
	end;
	};
end;

return t;
