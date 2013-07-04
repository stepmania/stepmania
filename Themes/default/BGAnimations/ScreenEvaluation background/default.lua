local t = Def.ActorFrame {};

t[#t+1] = Def.ActorFrame {
	InitCommand=function(self)
		self:Center();
	end;
	Def.Sprite {
		OnCommand=function(self)
			if GAMESTATE:GetCurrentSong() then
				local song = GAMESTATE:GetCurrentSong();
				if song:HasBackground() then
					self:LoadBackground(song:GetBackgroundPath());
				end;
				self:scale_or_crop_background()
				self:fadebottom(0.25)
				self:fadetop(0.25)
				self:croptop(1/10)
				self:cropbottom(1/10)
			else
				self:visible(false);
			end
		end;
	};
 	Def.Quad {
		InitCommand=function(self)
			self:scaletoclipped(SCREEN_WIDTH + 1, SCREEN_HEIGHT);
		end;
		OnCommand=function(self)
			self:diffuse(color("#FFCB05"));
			self:diffusebottomedge(color("#F0BA00"));
			self:diffusealpha(0.45);
		end;
	};
	LoadActor(THEME:GetPathB("ScreenWithMenuElements","background/_bg top")) .. {
		InitCommand=function(self)
			self:scaletoclipped(SCREEN_WIDTH + 1, SCREEN_HEIGHT);
		end;
	};

};

return t