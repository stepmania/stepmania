return LoadFont("ScreenGameplay","SongTitle") .. {
	CurrentSongChangedMessageCommand=cmd(playcommand,"Refresh");
	RefreshCommand=function(self)
		local vSong = GAMESTATE:GetCurrentSong();
		local vCourse = GAMESTATE:GetCurrentCourse();
		local sText = ""
		if vSong then
			sText = vSong:GetDisplayArtist() .. " - " .. vSong:GetDisplayFullTitle()
		end
		if vCourse then
			sText = vCourse:GetDisplayFullTitle() .. " - " .. vSong:GetDisplayFullTitle();
		end
		self:settext( sText );
		self:playcommand( "On" );
	end;
};