local t = LoadFallbackB();

t[#t+1] = Def.ActorFrame {
	InitCommand=cmd(x,SCREEN_CENTER_X*1.375;y,SCREEN_CENTER_Y);
	SetCommand=function(self)
		local c = self:GetChildren();
		local SongOrCourse;
		local Info = { Title, Genre, Artist, Meter, BPMs };
		if GAMESTATE:GetCurrentSong() then
			SongOrCourse = GAMESTATE:GetCurrentSong();
		elseif GAMESTATE:GetCurrentCourse() then
			SongOrCourse = GAMESTATE:GetCurrentCourse();
		else
			return;
		end;
		--
		Info.Title = SongOrCourse:GetDisplayFullTitle();
		Info.Artist = SongOrCourse:GetDisplayArtist();
		Info.Genre = SongOrCourse:GetGenre();
		Info.Meter = GAMESTATE:GetCurrentSteps( GAMESTATE:GetMasterPlayerNumber() ):GetMeter();
		Info.BPMS = { 0, 0 };
		--
		c.Title:settext( Info.Title );
		-- Title Size Check.
		if c.Title:GetWidth() >= SCREEN_CENTER_X*1.375-48 then
			c.Title:zoom(0.75);
			c.Title:maxwidth( ( SCREEN_CENTER_X*1.375-48 ) / 0.75);
		else
			c.Title:zoom(1);
			c.Title:maxwidth(SCREEN_CENTER_X*1.375-48);
		end
--[[ 		-- Genre Text Check
		if Info.Genre == "" then
			Info.Genre = "?";
		else
			Info.Genre = Info.Genre;
		end; --]]
		c.Genre:settext( Info.Genre );
		--
		c.Artist:settext( Info.Artist );
		-- BPM Check
 		if string.find(tostring(c.BPMDisplay:GetText()),"-") ~= nil then
			Info.BPMS = split('-',c.BPMDisplay:GetText());
			c.BPMLow:settext( Info.BPMS[1] );
			c.BPMHigh:settext( Info.BPMS[2] );
			c.BPMLow:visible(true);
			--
			c.BPMHigh:x(0);
			c.BPMLow:x( -c.BPMHigh:GetWidth() );
			--
		else
			Info.BPMS[1] = c.BPMDisplay:GetText();
			c.BPMLow:settext( Info.BPMS[1] );
			c.BPMHigh:settext( Info.BPMS[1] );
			c.BPMLow:visible(false);
			--
			c.BPMHigh:x(0);
			c.BPMLow:x( -c.BPMHigh:GetWidth() );
			--
		end 
--[[ 		c.BPMLow:visible(false);
		c.BPMHigh:settext( c.BPMDisplay:GetText() ); --]]
		--
		c.Title:playcommand("Refresh");
		c.Genre:playcommand("Refresh");
		c.Artist:playcommand("Refresh");
	end;
	OnCommand=cmd(playcommand,"Set");
	CurrentCourseChangedMessageCommand=cmd(playcommand,"Set");
	CurrentSongChangedMessageCommand=cmd(playcommand,"Set");
	DisplayLanguageChangedMessageCommand=cmd(playcommand,"Set");

	Def.BPMDisplay {
		Name="BPMDisplay";
		File=THEME:GetPathF("BPMDisplay", "bpm");
		InitCommand=cmd(visible,false);
		CurrentSongChangedMessageCommand=cmd( SetFromGameState );
	};
	LoadFont("UI","Title Large") .. {
		Name="Title";
		Text="!!!";
		InitCommand=cmd(horizalign,right);
		RefreshCommand=cmd(finishtweening;diffuse,Color.Blue;decelerate,0.2;diffuse,Color.White);
-- 		ShowCommand=cmd(settext,"!!!";);
	};
	LoadFont("UI","Title Small") .. {
		Name="Genre";
		Text="!!!";
		InitCommand=cmd(y,-52/2-3;horizalign,right;);
		OnCommand=cmd(shadowlength,1);
-- 		ShowCommand=cmd(settext,"!!!";);
	};
	LoadFont("UI","Title Small") .. {
		Name="Artist";
		Text="!!!";
		InitCommand=cmd(y,52/2+3;horizalign,right;);
		OnCommand=cmd(shadowlength,1);
-- 		ShowCommand=cmd(settext,"!!!";);
	};
	LoadFont("UI","Title Small") .. {
		Name="BPMLow";
		Text="!!!";
		InitCommand=cmd(y,52/2+3+20;horizalign,right;);
		OnCommand=cmd(shadowlength,1;zoom,0.75);
-- 		ShowCommand=cmd(settext,"!!!";);
	};
	LoadFont("UI","Title Small") .. {
		Name="BPMHigh";
		Text="!!!";
		InitCommand=cmd(y,52/2+3+20;horizalign,right;);
		OnCommand=cmd(shadowlength,1);
-- 		ShowCommand=cmd(settext,"!!!";);
	};
};

return t