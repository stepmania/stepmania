return Def.ActorFrame{
	Font("mentone","24px")..{
		InitCommand=cmd(halign,1;shadowlength,0;zoom,0.5;strokecolor,color("0,0,0,0"));
		BeginCommand=cmd(playcommand,"Update");
		UpdateCommand=function(self)
			local selection;
			local seltime = 0;
			if GAMESTATE:IsCourseMode() then
				selection = GAMESTATE:GetCurrentCourse();
				if selection then
					if GAMESTATE:GetCurrentTrail(GAMESTATE:GetMasterPlayerNumber()) then
						local trail = GAMESTATE:GetCurrentTrail(GAMESTATE:GetMasterPlayerNumber());
						seltime = TrailUtil.GetTotalSeconds( trail );
					end;
				end;
			else
				selection = GAMESTATE:GetCurrentSong();
				if selection then
					seltime = selection:MusicLengthSeconds();
				end;
			end;

			-- todo: r21 ogglength patch thing?
			seltime = SecondsToMSS(seltime);
			self:settext(seltime);
		end;
		CurrentSongChangedMessageCommand=cmd(playcommand,"Update");
		CurrentCourseChangedMessageCommand=cmd(playcommand,"Update");
		CurrentTrailP1ChangedMessageCommand=cmd(playcommand,"Update");
		CurrentTrailP2ChangedMessageCommand=cmd(playcommand,"Update");
	};
};