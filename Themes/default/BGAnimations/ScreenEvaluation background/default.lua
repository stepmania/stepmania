local t = Def.ActorFrame {};

t[#t+1] = Def.ActorFrame {
  InitCommand=cmd(Center);
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
		InitCommand=cmd(scaletoclipped,SCREEN_WIDTH+1,SCREEN_HEIGHT);
		OnCommand=cmd(diffuse,color("#FFCB05");diffusebottomedge,color("#F0BA00");diffusealpha,0.45);
	};
--[[
	LoadActor(THEME:GetPathB("ScreenWithMenuElements","background/_grid")).. {
		InitCommand=cmd(customtexturerect,0,0,(SCREEN_WIDTH+1)/4,SCREEN_HEIGHT/4;SetTextureFiltering,true);
		OnCommand=cmd(zoomto,SCREEN_WIDTH+1,SCREEN_HEIGHT;diffuse,Color("Black");diffuseshift;effecttiming,(1/8)*4,0,(7/8)*4,0;effectclock,'beatnooffset';
		effectcolor2,Color("Black");effectcolor1,Colors.Alpha(Color("Black"),0.45);fadebottom,0.25;fadetop,0.25;croptop,48/480;cropbottom,48/480;diffusealpha,0.345);
	}; --]]
	LoadActor(THEME:GetPathB("ScreenWithMenuElements","background/_bg top")) .. {
		InitCommand=cmd(scaletoclipped,SCREEN_WIDTH+1,SCREEN_HEIGHT);
	};

};

return t