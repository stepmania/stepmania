local t = Def.ActorFrame{
	Def.Quad{
		Name="TopFrame";
		InitCommand=function(self)
			self:CenterX();
			self:y(SCREEN_TOP);
			self:valign(0);
			self:zoomto(SCREEN_WIDTH, SCREEN_HEIGHT * 0.15);
			self:diffuse(color("0,0,0,1"));
			self:fadebottom(0.5);
		end;
	};

	Def.Quad{
		Name="BotFrame";
		InitCommand=function(self)
			self:CenterX();
			self:y(SCREEN_BOTTOM);
			self:valign(1);
			self:zoomto(SCREEN_WIDTH, SCREEN_HEIGHT * 0.15);
			self:diffuse(color("0,0,0,1"));
			self:fadetop(0.5);
		end;
	};

	Def.ActorFrame{
		Name="MiddleSection";
		InitCommand=function(self)
			self:CenterX();
			self:y(SCREEN_CENTER_Y * 1.35);
		end;
		Def.Quad{
			Name="Frame";
			InitCommand=function(self)
				self:zoomto(SCREEN_WIDTH, 0);
				self:diffuse(color("0.1,0.1,0.1,0.75"));
				self:fadebottom(0.25);
				self:fadetop(0.25);
			end;
			OnCommand=function(self)
				self:smooth(0.75);
				self:zoomtoheight(64);
			end;
		};
		LoadFont("Common normal")..{
			Text=Screen.String("Demonstration");
			InitCommand=function(self)
				self:diffusealpha(0);
				self:strokecolor(color("0,0,0,0.5"));
			end;
			OnCommand=function(self)
				self:smooth(0.75);
				self:diffusealpha(1);
				self:diffuseshift();
				self:effectcolor1(HSV(38,0.45,0.95));
			end;
		};
	};

	LoadFont("Common normal")..{
		Name="SongTitle";
		InitCommand=function(self)
			self:x(SCREEN_CENTER_X);
			self:y(SCREEN_TOP+16);
			self:zoom(0.5);
			self:shadowlength(1);
			self:valign(0);
		end;
		BeginCommand=function(self)
			local song = GAMESTATE:GetCurrentSong();
			local text = "";
			if song then
				local artist = song:GetDisplayArtist()
				local title = song:GetDisplayFullTitle()
				local group = song:GetGroupName()
				text = string.format(Screen.String("%s - %s   [from %s]"),artist,title,group)
			end;
			self:settext(text);
		end;
	};
};

return t;