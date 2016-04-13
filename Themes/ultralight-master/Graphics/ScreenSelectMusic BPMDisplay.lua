local t = Def.ActorFrame{
	Def.BPMDisplay {
		--File=FontPath("mentone","24px");
		File=THEME:GetPathF("Common", "numbers");
		Name="BPMDisplay";
		InitCommand=cmd(y,-1;zoom,0.8;vertalign,bottom;NoStroke;shadowlength,1;maxwidth,SCREEN_CENTER_X*0.75);
		SetCommand=function(self)
			self:SetFromGameState();
		end;
		CurrentSongChangedMessageCommand=cmd(playcommand,"Set");
		CurrentCourseChangedMessageCommand=cmd(playcommand,"Set");
	};
	Font("mentone","24px") .. {
		Name="Label";
		InitCommand=cmd(y,1;zoom,0.8;vertalign,top;NoStroke;shadowlength,1;maxwidth,SCREEN_CENTER_X*0.75;settext,"BPM");
		SetCommand=function(self)
			if GAMESTATE:GetCurrentSong() then
				-- song-related funnery here
				local timing = GAMESTATE:GetCurrentSong():GetTimingData();
				self:stoptweening();
				if timing:HasStops() then
					self:linear(0.125);
					self:diffusebottomedge( HSV(192,0.8,0.75) );
				else
					self:linear(0.125);
					self:diffusebottomedge( HSV(192,0,1) );
				end;
			else
				-- do this in case the previous song had a freeze
				self:linear(0.125);
				self:diffusebottomedge( HSV(192,0,1) );
			end;
		end;
		CurrentSongChangedMessageCommand=cmd(playcommand,"Set");
	};
};

return t;