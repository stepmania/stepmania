local Color1 = color(Var "Color1");
local Color2 = color(Var "Color2");

local a1 = LoadActor(Var "File1") .. {
	OnCommand= function(self)
		self:cropto(SCREEN_WIDTH/2,SCREEN_HEIGHT/2):diffuse(Color1)
			:effectclock("music")
		-- Explanation in StretchNoLoop.lua.
		if self.GetTexture then
			self:GetTexture():rate(self:GetParent():GetUpdateRate())
		end
	end,
	GainFocusCommand=cmd(play);
	LoseFocusCommand=cmd(pause);
};
local a2 = LoadActor(Var "File2") .. {
	OnCommand= function(self)
		self:cropto(SCREEN_WIDTH/2,SCREEN_HEIGHT/2):diffuse(Color2):effectclock("music")
		-- Explanation in StretchNoLoop.lua.
		if self.GetTexture then
			self:GetTexture():rate(self:GetParent():GetUpdateRate())
		end
	end,
	GainFocusCommand=cmd(play);
	LoseFocusCommand=cmd(pause);
};

local t = Def.ActorFrame {
	a1 .. { OnCommand=cmd(x,scale(1,0,4,SCREEN_LEFT,SCREEN_RIGHT);y,scale(1,0,4,SCREEN_TOP,SCREEN_BOTTOM)); };
	a2 .. { OnCommand=cmd(x,scale(3,0,4,SCREEN_LEFT,SCREEN_RIGHT);y,scale(1,0,4,SCREEN_TOP,SCREEN_BOTTOM)); };
	a2 .. { OnCommand=function(self)
						self:x(scale(1,0,4,SCREEN_LEFT,SCREEN_RIGHT)):y(scale(3,0,4,SCREEN_TOP,SCREEN_BOTTOM))
						if self.SetDecodeMovie then self:SetDecodeMovie(false) end
				end };
	a1 .. { OnCommand=function(self)
						self:x(scale(3,0,4,SCREEN_LEFT,SCREEN_RIGHT)):y(scale(3,0,4,SCREEN_TOP,SCREEN_BOTTOM))
						if self.SetDecodeMovie then self:SetDecodeMovie(false) end
				end };
};

return t;

