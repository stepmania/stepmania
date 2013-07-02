--- Stretch a file with green diffusion.
local Color1 = color("0,1,0,1");

local t = Def.ActorFrame {
	LoadActor(Var "File1") .. {
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
};

return t;
