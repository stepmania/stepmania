local t = Def.ActorFrame {};
t[#t+1] = Def.Quad {
	InitCommand=cmd(Center;zoomto,SCREEN_WIDTH,SCREEN_HEIGHT;diffuse,Color("Black"));
};
t[#t+1] = Def.Sprite {
	InitCommand=cmd(Center);
	BeginCommand=function(self)
		if not GAMESTATE:IsCourseMode() then
			self:LoadBackground( GAMESTATE:GetCurrentSong():GetBackgroundPath() );
		end;
	end;
	OnCommand=cmd(diffusealpha,0;scaletoclipped,SCREEN_WIDTH,SCREEN_HEIGHT;sleep,0.5;linear,0.50;diffusealpha,PREFSMAN:GetPreference("BGBrightness"));
};

t[#t+1] = Def.ActorFrame {
  InitCommand=cmd(Center);
  OnCommand=cmd(stoptweening;addx,30;linear,3;addx,-30);
	LoadFont("Common Normal") .. {
		Text=GAMESTATE:IsCourseMode() and GAMESTATE:GetCurrentCourse():GetDisplayFullTitle() or GAMESTATE:GetCurrentSong():GetDisplayFullTitle();
		InitCommand=cmd(strokecolor,Color("Outline");y,-20);
		OnCommand=cmd(faderight,1;diffusealpha,0;linear,0.5;faderight,0;diffusealpha,1;sleep,1.5;linear,0.5;diffusealpha,0);
	};
	LoadFont("Common Normal") .. {
		Text=GAMESTATE:IsCourseMode() and ToEnumShortString( GAMESTATE:GetCurrentCourse():GetCourseType() ) or GAMESTATE:GetCurrentSong():GetDisplayArtist();
		InitCommand=cmd(strokecolor,Color("Outline");zoom,0.75);
		OnCommand=cmd(faderight,1;diffusealpha,0;linear,0.5;faderight,0;diffusealpha,1;sleep,1.5;linear,0.5;diffusealpha,0);
	};
	LoadFont("Common Normal") .. {
		InitCommand=cmd(strokecolor,Color("Outline");diffuse,Color("Orange");diffusebottomedge,Color("Yellow");zoom,0.75;y,20);
		BeginCommand=function(self)
			local text = "";
			local SongOrCourse;
			if GAMESTATE:IsCourseMode() then
				local trail = GAMESTATE:GetCurrentTrail(GAMESTATE:GetMasterPlayerNumber());
				SongOrCourse = GAMESTATE:GetCurrentCourse();
				if SongOrCourse:GetEstimatedNumStages() == 1 then
					text = SongOrCourse:GetEstimatedNumStages() .." Stage / ".. SecondsToMSSMsMs( TrailUtil.GetTotalSeconds(trail) );
				else
					text = SongOrCourse:GetEstimatedNumStages() .." Stages / ".. SecondsToMSSMsMs( TrailUtil.GetTotalSeconds(trail) );
				end
			else
				SongOrCourse = GAMESTATE:GetCurrentSong();
				text = SecondsToMSSMsMs( SongOrCourse:MusicLengthSeconds() );
			end;
			self:settext(text);
		end;
		OnCommand=cmd(faderight,1;diffusealpha,0;linear,0.5;faderight,0;diffusealpha,1;sleep,1.5;linear,0.5;diffusealpha,0);
	};
};

return t