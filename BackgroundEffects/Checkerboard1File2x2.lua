local Color1 = color(Var "Color1");

local a = LoadActor(Var "File1") .. {
	OnCommand= function(self)
		self:cropto(_screen.w/2, _screen.h/2):diffuse(Color1)
			:effectclock("music")
		-- Explanation in StretchNoLoop.lua.
		if self.GetTexture then
			self:GetTexture():rate(self:GetParent():GetUpdateRate())
		end
	end,
	GainFocusCommand=cmd(play);
	LoseFocusCommand=cmd(pause);
};

local t = Def.ActorFrame {
	a .. { OnCommand=cmd(x,scale(1,0,4,SCREEN_LEFT,SCREEN_RIGHT);y,scale(1,0,4,SCREEN_TOP,SCREEN_BOTTOM)); };
	a .. { OnCommand=function(self)
					 self:x(scale(3,0,4,SCREEN_LEFT,SCREEN_RIGHT)):y(scale(1,0,4,SCREEN_TOP,SCREEN_BOTTOM))
					 if self.SetDecodeMovie then self:SetDecodeMovie(false) end
			 end };
	a .. { OnCommand=function(self)
					 self:x(scale(1,0,4,SCREEN_LEFT,SCREEN_RIGHT)):y(scale(3,0,4,SCREEN_TOP,SCREEN_BOTTOM))
					 if self.SetDecodeMovie then self:SetDecodeMovie(false) end
			 end };
	a .. { OnCommand=function(self)
					 self:x(scale(3,0,4,SCREEN_LEFT,SCREEN_RIGHT)):y(scale(3,0,4,SCREEN_TOP,SCREEN_BOTTOM))
					 if self.SetDecodeMovie then self:SetDecodeMovie(false) end
			 end };
};

return t;
