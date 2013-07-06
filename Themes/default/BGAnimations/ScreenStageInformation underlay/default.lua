local playMode = GAMESTATE:GetPlayMode()

local sStage = ""
sStage = GAMESTATE:GetCurrentStage()

if playMode ~= 'PlayMode_Regular' and playMode ~= 'PlayMode_Rave' and playMode ~= 'PlayMode_Battle' then
  sStage = playMode;
end;

if not (GAMESTATE:IsCourseMode() or GAMESTATE:IsExtraStage() or GAMESTATE:IsExtraStage2()) then
	local tRemap = {
		Stage_Event		= 0,
		Stage_1st		= 1,
		Stage_2nd		= 2,
		Stage_3rd		= 3,
		Stage_4th		= 4,
		Stage_5th		= 5,
		Stage_6th		= 6,
	};

	local nSongCount = tRemap[sStage] + (GAMESTATE:GetCurrentSong():GetStageCost()-1);

	if nSongCount >= PREFSMAN:GetPreference("SongsPerPlay") then
	sStage = "Stage_Final";
	else
		sStage = sStage;
	end;
end;

local t = Def.ActorFrame {};
t[#t+1] = Def.Quad {
	InitCommand=function(self)
		self:Center();
		self:zoomto(SCREEN_WIDTH, SCREEN_HEIGHT);
		self:diffuse(Color("Black"));
	end;
};
if GAMESTATE:IsCourseMode() then
	t[#t+1] = LoadActor("CourseDisplay");
else
	t[#t+1] = Def.Sprite {
		InitCommand=function(self)
			self:Center();
			self:diffusealpha(0);
		end;
		BeginCommand=function(self)
			self:LoadFromCurrentSongBackground();
		end;
		OnCommand=function(self)
			if PREFSMAN:GetPreference("StretchBackgrounds") then
				self:SetSize(SCREEN_WIDTH,SCREEN_HEIGHT)
			else
				self:scale_or_crop_background()
			end
			self:sleep(0.5)
			self:linear(0.50)
			self:diffusealpha(1)
			self:sleep(3)
		end;
	};
end

t[#t+1] = Def.ActorFrame {
	InitCommand=function(self)
		self:x(SCREEN_CENTER_X);
		self:y(SCREEN_CENTER_Y);
	end;
	OnCommand=function(self)
		self:stoptweening();
		self:zoom(1.25);
		self:decelerate(3);
		self:zoom(1);
	end;
	
	LoadActor( THEME:GetPathG("ScreenStageInformation", "Stage " .. ToEnumShortString(sStage) ) ) .. {
		OnCommand=function(self)
			self:diffusealpha(0);
			self:linear(0.25);
			self:diffusealpha(1);
			self:sleep(1.75);
			self:linear(0.5);
			self:zoomy(0);
			self:zoomx(2);
			self:diffusealpha(0);
		end;
	};
};

t[#t+1] = Def.ActorFrame {
	InitCommand=function(self)
		self:x(SCREEN_CENTER_X);
		self:y(SCREEN_CENTER_Y + 96);
	end;
	OnCommand=function(self)
		self:stoptweening();
		self:addy(-16);
		self:decelerate(3);
		self:addy(16);
	end;
	LoadFont("Common Normal") .. {
		Text=GAMESTATE:IsCourseMode() and GAMESTATE:GetCurrentCourse():GetDisplayFullTitle() or GAMESTATE:GetCurrentSong():GetDisplayFullTitle();
		InitCommand=function(self)
			self:strokecolor(Color("Outline"));
			self:y(-20);
		end;
		OnCommand=function(self)
			self:diffusealpha(0);
			self:linear(0.5);
			self:diffusealpha(1);
			self:sleep(1.5);
			self:linear(0.5);
			self:diffusealpha(0);
		end;
	};
	LoadFont("Common Normal") .. {
		Text=GAMESTATE:IsCourseMode() and ToEnumShortString( GAMESTATE:GetCurrentCourse():GetCourseType() ) or GAMESTATE:GetCurrentSong():GetDisplayArtist();
		InitCommand=function(self)
			self:strokecolor(Color("Outline"));
			self:zoom(0.75);
		end;
		OnCommand=function(self)
			self:diffusealpha(0);
			self:linear(0.5);
			self:diffusealpha(1);
			self:sleep(1.5);
			self:linear(0.5);
			self:diffusealpha(0);
		end;
	};
	LoadFont("Common Normal") .. {
		InitCommand=function(self)
			self:strokecolor(Color("Outline"));
			self:diffuse(Color("Orange"));
			self:diffusebottomedge(Color("Yellow"));
			self:zoom(0.75);
			self:y(20);
		end;
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
		OnCommand=function(self)
			self:diffusealpha(0);
			self:linear(0.5);
			self:diffusealpha(1);
			self:sleep(1.5);
			self:linear(0.5);
			self:diffusealpha(0);
		end;
	};
};

return t
