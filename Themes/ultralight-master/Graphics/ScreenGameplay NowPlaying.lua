local t = Def.ActorFrame{
	Font("mentone","24px") .. {
		Name="Genre";
		InitCommand=cmd(zoom,0.5;NoStroke;shadowlength,1;maxwidth,784);
		OnCommand=cmd(playcommand,"Set");
		SetCommand=function(self)
			local text = "";
			if GAMESTATE:IsCourseMode() then
				-- current song [course] (cur#/#songs)
				if GAMESTATE:GetCurrentCourse() then
					-- xxx: have to deal with endless
					local songIdx = GAMESTATE:GetCourseSongIndex();
					local curSong = GAMESTATE:GetCurrentCourse():GetCourseEntry(songIdx):GetSong();
					if curSong then
						text = string.format(
							ScreenString("%s [%s] (Song %i/%i)"),
							curSong:GetDisplayFullTitle(),
							GAMESTATE:GetCurrentCourse():GetDisplayFullTitle(),
							songIdx+1,
							GAMESTATE:GetCurrentCourse():GetEstimatedNumStages()
						);
					else
						text = string.format(
							ScreenString("%s (Song %i/%i)"),
							GAMESTATE:GetCurrentCourse():GetDisplayFullTitle(),
							songIdx+1,
							GAMESTATE:GetCurrentCourse():GetEstimatedNumStages()
						);
					end;
				end;
			else
				if GAMESTATE:GetCurrentSong() then
					text = string.format(
						"%s / %s",
						GAMESTATE:GetCurrentSong():GetDisplayFullTitle(),
						GAMESTATE:GetCurrentSong():GetDisplayArtist()
					);
				end;
			end;
			self:settext( text );
		end;
		CurrentSongChangedMessageCommand=cmd(playcommand,"Set");
	};
};

return t;