local t = Def.ActorFrame {};

t[#t+1] = Def.ActorFrame {
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
		InitCommand=cmd(Center;scaletoclipped,SCREEN_WIDTH+1,SCREEN_HEIGHT);
		OnCommand=cmd(diffuse,color("#FFCB05");diffusebottomedge,color("#F0BA00");diffusealpha,0.45);
	};
};

return t
