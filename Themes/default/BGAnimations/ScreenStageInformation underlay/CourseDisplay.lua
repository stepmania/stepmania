if not GAMESTATE:IsCourseMode() then return Def.ActorFrame{} end; -- short circuit
local course = GAMESTATE:GetCurrentCourse()

local t = Def.ActorFrame{
	-- background
	Def.Sprite{
		InitCommand=function(self)
			self:(Center);
		end;
		BeginCommand=function(self)
			if course:GetBackgroundPath() then
				self:Load( course:GetBackgroundPath() )
			else
				-- default to the BG of the first song in the course
				self:LoadFromCurrentSongBackground()
			end
		end;
		OnCommand=function(self)
			self:diffusealpha(0);
			self:scale_or_crop_background();
			self:sleep(0.5);
			self:linear(0.50);
			self:diffusealpha(1);
			self:sleep(3);
		end;
	};
	-- alternate background
	Def.Sprite{
		InitCommand=function(self)
			self:(Center);
		end;
		BeginCommand=function(self)
			self:LoadFromCurrentSongBackground();
			self:scale_or_crop_background();
			self:diffusealpha(0);
		OnCommand=function(self)
			self:sleep(4);
			self:playcommand("Show");
		end;
		ShowCommand=function(self)
			if course:HasBackground() then
				self:accelerate(0.25)
				self:diffusealpha(1)
			end
		end;
	};
};

return t;