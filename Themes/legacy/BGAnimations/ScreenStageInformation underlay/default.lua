local playMode = GAMESTATE:GetPlayMode()

local sStage = ""
sStage = GAMESTATE:GetCurrentStage()

if playMode ~= 'PlayMode_Regular' and playMode ~= 'PlayMode_Rave' and playMode ~= 'PlayMode_Battle' then
  sStage = playMode;
end;

local t = Def.ActorFrame {};
t[#t+1] = Def.Quad {
	InitCommand=cmd(Center;zoomto,SCREEN_WIDTH,SCREEN_HEIGHT;diffuse,Color("Black"));
};
if GAMESTATE:IsCourseMode() then
	t[#t+1] = LoadActor("CourseDisplay");
else
	t[#t+1] = Def.Sprite {
		InitCommand=cmd(Center;diffusealpha,0);
		BeginCommand=cmd(LoadFromCurrentSongBackground);
		OnCommand=function(self)
			self:scale_or_crop_background()
			self:sleep(0.5)
			self:linear(0.50)
			self:diffusealpha(1)
			self:sleep(3)
		end;
	};
end

local stage_num_actor= THEME:GetPathG("ScreenStageInformation", "Stage " .. ToEnumShortString(sStage), true)
if stage_num_actor ~= "" and FILEMAN:DoesFileExist(stage_num_actor) then
	stage_num_actor= LoadActor(stage_num_actor)
else
	-- Midiman:  We need a "Stage Next" actor or something for stages after
	-- the 6th. -Kyz
	local curStage = GAMESTATE:GetCurrentStage();
	stage_num_actor= Def.BitmapText{
		Font= "Common Normal",  Text= thified_curstage_index(false) .. " Stage",
		InitCommand= function(self)
			self:zoom(1.5)
			self:strokecolor(Color.Black)
			self:diffuse(StageToColor(curStage));
			self:diffusetopedge(ColorLightTone(StageToColor(curStage)));
		end
	}
end

t[#t+1] = Def.ActorFrame {
	InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y);
	OnCommand=cmd(stoptweening;zoom,1.25;decelerate,3;zoom,1);
	stage_num_actor .. {
		OnCommand=cmd(diffusealpha,0;linear,0.25;diffusealpha,1;sleep,1.75;linear,0.5;zoomy,0;zoomx,2;diffusealpha,0);
	};
};

t[#t+1] = Def.ActorFrame {
  InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y+96);
  OnCommand=cmd(stoptweening;addy,-16;decelerate,3;addy,16);
	LoadFont("Common Normal") .. {
		Text=GAMESTATE:IsCourseMode() and GAMESTATE:GetCurrentCourse():GetDisplayFullTitle() or GAMESTATE:GetCurrentSong():GetDisplayFullTitle();
		InitCommand=cmd(strokecolor,Color("Outline");y,-20);
		OnCommand=cmd(diffusealpha,0;linear,0.5;diffusealpha,1;sleep,1.5;linear,0.5;diffusealpha,0);
	};
	LoadFont("Common Normal") .. {
		Text=GAMESTATE:IsCourseMode() and ToEnumShortString( GAMESTATE:GetCurrentCourse():GetCourseType() ) or GAMESTATE:GetCurrentSong():GetDisplayArtist();
		InitCommand=cmd(strokecolor,Color("Outline");zoom,0.75);
		OnCommand=cmd(diffusealpha,0;linear,0.5;diffusealpha,1;sleep,1.5;linear,0.5;diffusealpha,0);
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
		OnCommand=cmd(diffusealpha,0;linear,0.5;diffusealpha,1;sleep,1.5;linear,0.5;diffusealpha,0);
	};
};

return t
