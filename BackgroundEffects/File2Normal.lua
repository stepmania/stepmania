local Color1 = color(Var "Color1");
local Color2 = color(Var "Color2");

local t = Def.ActorFrame {};

t[#t+1] = LoadActor(Var "File1") .. {
	OnCommand=function(self)
self:x(SCREEN_CENTER_X):y(SCREEN_CENTER_Y):scale_or_crop_background():diffuse(Color1):effectclock("music")
		-- Explanation in StretchNoLoop.lua.
		if self.GetTexture then
			self:GetTexture():rate(self:GetParent():GetUpdateRate())
		end
	end,
	GainFocusCommand=cmd(play);
	LoseFocusCommand=cmd(pause);
};

if Var("File2") ~= nil then
	t[#t+1] = LoadActor(Var("File2")) .. {
		OnCommand=function(self)
			self:x(SCREEN_CENTER_X):y(SCREEN_CENTER_Y):scale_or_crop_background():diffuse(Color1):effectclock("music")
			-- Explanation in StretchNoLoop.lua.
			if self.GetTexture then
				self:GetTexture():rate(self:GetParent():GetUpdateRate())
			end
		end,
		GainFocusCommand=cmd(play);
		LoseFocusCommand=cmd(pause);
	};
end;

return t;
