local curStage = GAMESTATE:GetCurrentStage();
local curStageIndex = GAMESTATE:GetCurrentStageIndex();
local t = LoadFallbackB();

local function PercentScore(pn)
	local t = LoadFont("_overpass Score")..{
		InitCommand=cmd(zoom,1;diffuse,Color("Black");diffusealpha,0.75);
		BeginCommand=cmd(playcommand,"Set");
		SetCommand=function(self)
			local SongOrCourse, StepsOrTrail;
			if GAMESTATE:IsCourseMode() then
				SongOrCourse = GAMESTATE:GetCurrentCourse();
				StepsOrTrail = GAMESTATE:GetCurrentTrail(pn);
			else
				SongOrCourse = GAMESTATE:GetCurrentSong();
				StepsOrTrail = GAMESTATE:GetCurrentSteps(pn);
			end;

			local profile, scorelist;
			local text = "";
			if SongOrCourse and StepsOrTrail then
				local st = StepsOrTrail:GetStepsType();
				local diff = StepsOrTrail:GetDifficulty();
				local courseType = GAMESTATE:IsCourseMode() and SongOrCourse:GetCourseType() or nil;
				local cd = GetCustomDifficulty(st, diff, courseType);

				if PROFILEMAN:IsPersistentProfile(pn) then
					-- player profile
					profile = PROFILEMAN:GetProfile(pn);
				else
					-- machine profile
					profile = PROFILEMAN:GetMachineProfile();
				end;

				scorelist = profile:GetHighScoreList(SongOrCourse,StepsOrTrail);
				assert(scorelist)
				local scores = scorelist:GetHighScores();
				local topscore = scores[1];
				if topscore then
					text = string.format("%.2f%%", topscore:GetPercentDP()*100.0);
					-- 100% hack
					if text == "100.00%" then
						text = "100%";
					end;
				else
					text = string.format("%.2f%%", 0);
				end;
			else
				text = "";
			end;
			self:settext(text);
		end;
		CurrentSongChangedMessageCommand=cmd(playcommand,"Set");
		CurrentCourseChangedMessageCommand=cmd(playcommand,"Set");
	};

	if pn == PLAYER_1 then
		t.CurrentStepsP1ChangedMessageCommand=cmd(playcommand,"Set");
		t.CurrentTrailP1ChangedMessageCommand=cmd(playcommand,"Set");
	else
		t.CurrentStepsP2ChangedMessageCommand=cmd(playcommand,"Set");
		t.CurrentTrailP2ChangedMessageCommand=cmd(playcommand,"Set");
	end

	return t;
end

-- Genre/Artist data
t[#t+1] = LoadActor(THEME:GetPathG("ScreenSelectMusic", "info pane")) .. {
		InitCommand=cmd(horizalign,center;x,SCREEN_CENTER_X-228;y,SCREEN_CENTER_Y-75;zoom,1);
		OnCommand=function(self)
			self:diffuse(ColorMidTone(StageToColor(curStage)));
			self:zoomx(0):diffusealpha(0):decelerate(0.3):zoomx(1):diffusealpha(1);
		end;
		OffCommand=function(self)
			self:sleep(0.3):decelerate(0.15):zoomx(0):diffusealpha(0);
		end;
		};

t[#t+1] = Def.ActorFrame {
    InitCommand=cmd(x,SCREEN_CENTER_X-330+6-138;draworder,126);
    OnCommand=cmd(diffusealpha,0;smooth,0.3;diffusealpha,1);
    OffCommand=cmd(smooth,0.3;diffusealpha,0);
    -- Length
	StandardDecorationFromFileOptional("SongTime","SongTime") .. {
	SetCommand=function(self)
		local curSelection = nil;
		local length = 0.0;
		if GAMESTATE:IsCourseMode() then
			curSelection = GAMESTATE:GetCurrentCourse();
			self:queuecommand("Reset");
			if curSelection then
				local trail = GAMESTATE:GetCurrentTrail(GAMESTATE:GetMasterPlayerNumber());
				if trail then
					length = TrailUtil.GetTotalSeconds(trail);
				else
					length = 0.0;
				end;
			else
				length = 0.0;
			end;
		else
			curSelection = GAMESTATE:GetCurrentSong();
			self:queuecommand("Reset");
			if curSelection then
				length = curSelection:MusicLengthSeconds();
				if curSelection:IsLong() then
					self:queuecommand("Long");
				elseif curSelection:IsMarathon() then
					self:queuecommand("Marathon");
				else
					self:queuecommand("Reset");
				end
			else
				length = 0.0;
				self:queuecommand("Reset");
			end;
		end;
		self:settext( SecondsToMSS(length) );
	end;
    	CurrentSongChangedMessageCommand=cmd(queuecommand,"Set");
    	CurrentCourseChangedMessageCommand=cmd(queuecommand,"Set");
    	CurrentTrailP1ChangedMessageCommand=cmd(queuecommand,"Set");
    	CurrentTrailP2ChangedMessageCommand=cmd(queuecommand,"Set");
    };
};

-- Course count and type
t[#t+1] = Def.ActorFrame {
    InitCommand=cmd(x,SCREEN_CENTER_X-200;draworder,126);
    OnCommand=cmd(diffusealpha,0;smooth,0.3;diffusealpha,1);
    OffCommand=cmd(smooth,0.2;diffusealpha,0);
	LoadFont("Common Condensed") .. { 
          InitCommand=cmd(horizalign,right;zoom,1.0;y,SCREEN_CENTER_Y-78+2;maxwidth,180;diffuse,color("#DFE2E9");visible,GAMESTATE:IsCourseMode());
          CurrentCourseChangedMessageCommand=cmd(queuecommand,"Set"); 
          ChangedLanguageDisplayMessageCommand=cmd(queuecommand,"Set"); 
          SetCommand=function(self) 
               local course = GAMESTATE:GetCurrentCourse(); 
               if course then
                    self:settext(course:GetEstimatedNumStages() .. " songs"); 
                    self:queuecommand("Refresh");
				else
					self:settext("");
					self:queuecommand("Refresh"); 	
               end 
          end; 
		};
};
t[#t+1] = Def.ActorFrame {
    InitCommand=cmd(x,SCREEN_CENTER_X+5;draworder,126);
    OnCommand=cmd(diffusealpha,0;smooth,0.3;diffusealpha,1);
    OffCommand=cmd(smooth,0.2;diffusealpha,0);
	LoadFont("Common Condensed") .. { 
          InitCommand=cmd(horizalign,right;zoom,1.0;y,SCREEN_CENTER_Y-78+2;maxwidth,180;diffuse,color("#DFE2E9");visible,GAMESTATE:IsCourseMode());
          CurrentCourseChangedMessageCommand=cmd(queuecommand,"Set"); 
          ChangedLanguageDisplayMessageCommand=cmd(queuecommand,"Set"); 
          SetCommand=function(self) 
               local course = GAMESTATE:GetCurrentCourse(); 
               if course then
                    self:settext(CourseTypeToLocalizedString(course:GetCourseType())); 
                    self:queuecommand("Refresh");
				else
					self:settext("");
					self:queuecommand("Refresh"); 	
               end 
          end; 
		};
};


if not GAMESTATE:IsCourseMode() then

-- P1 Difficulty Pane
t[#t+1] = Def.ActorFrame {
		InitCommand=cmd(visible,GAMESTATE:IsHumanPlayer(PLAYER_1);horizalign,center;x,SCREEN_CENTER_X-210-32;y,SCREEN_CENTER_Y+230+8);
		OnCommand=cmd(zoomy,0.8;diffusealpha,0;smooth,0.4;diffusealpha,1;zoomy,1);
		PlayerJoinedMessageCommand=function(self,param)
			if param.Player == PLAYER_1 then
				(cmd(visible,true;diffusealpha,0;linear,0.3;diffusealpha,1))(self);
			end;
		end;
		OffCommand=cmd(decelerate,0.3;zoomy,0.8;diffusealpha,0);
		LoadActor(THEME:GetPathG("ScreenSelectMusic", "pane background")) .. {
			CurrentStepsP1ChangedMessageCommand=cmd(queuecommand,"Set"); 
			PlayerJoinedMessageCommand=cmd(queuecommand,"Set";diffusealpha,0;decelerate,0.3;diffusealpha,1);
			ChangedLanguageDisplayMessageCommand=cmd(queuecommand,"Set");
			SetCommand=function(self)
					stepsP1 = GAMESTATE:GetCurrentSteps(PLAYER_1)
					local song = GAMESTATE:GetCurrentSong();
					if song then 
						if stepsP1 ~= nil then
							local st = stepsP1:GetStepsType();
							local diff = stepsP1:GetDifficulty();
							local courseType = GAMESTATE:IsCourseMode() and SongOrCourse:GetCourseType() or nil;
							local cd = GetCustomDifficulty(st, diff, courseType);
							self:finishtweening():linear(0.2):diffuse(ColorLightTone(CustomDifficultyToColor(cd)));
						else
							self:diffuse(color("#666666"));
						end
					else
							self:diffuse(color("#666666"));
					end
				  end
		};
		LoadFont("StepsDisplay meter") .. { 
			  InitCommand=cmd(zoom,1.25;diffuse,color("#000000");addx,-143;addy,13);
			  OnCommand=cmd(diffusealpha,0;smooth,0.2;diffusealpha,0.75);
			  OffCommand=cmd(linear,0.3;diffusealpha,0);
			  CurrentStepsP1ChangedMessageCommand=cmd(queuecommand,"Set"); 
			  PlayerJoinedMessageCommand=cmd(queuecommand,"Set";diffusealpha,0;linear,0.3;diffusealpha,0.75);
			  ChangedLanguageDisplayMessageCommand=cmd(queuecommand,"Set");
			  SetCommand=function(self)
				stepsP1 = GAMESTATE:GetCurrentSteps(PLAYER_1)
				local song = GAMESTATE:GetCurrentSong();
				if song then 
					if stepsP1 ~= nil then
						local st = stepsP1:GetStepsType();
						local diff = stepsP1:GetDifficulty();
						local courseType = GAMESTATE:IsCourseMode() and SongOrCourse:GetCourseType() or nil;
						local cd = GetCustomDifficulty(st, diff, courseType);
						self:settext(stepsP1:GetMeter())
					else
						self:settext("")
					end
				else
					self:settext("")
				end
			  end
		};
		LoadFont("Common Italic Condensed") .. { 
			  InitCommand=cmd(uppercase,true;zoom,1;addy,-40;addx,-143;diffuse,color("#000000");maxwidth,115);
			  OnCommand=cmd(diffusealpha,0;smooth,0.2;diffusealpha,0.75);
			  OffCommand=cmd(linear,0.3;diffusealpha,0);
			  CurrentStepsP1ChangedMessageCommand=cmd(queuecommand,"Set"); 
			  PlayerJoinedMessageCommand=cmd(queuecommand,"Set";diffusealpha,0;linear,0.3;diffusealpha,0.75);
			  ChangedLanguageDisplayMessageCommand=cmd(queuecommand,"Set");
			  SetCommand=function(self)
				stepsP1 = GAMESTATE:GetCurrentSteps(PLAYER_1)
				local song = GAMESTATE:GetCurrentSong();
				if song then 
					if stepsP1 ~= nil then
						local st = stepsP1:GetStepsType();
						local diff = stepsP1:GetDifficulty();
						local courseType = GAMESTATE:IsCourseMode() and SongOrCourse:GetCourseType() or nil;
						local cd = GetCustomDifficulty(st, diff, courseType);
						self:settext(THEME:GetString("CustomDifficulty",ToEnumShortString(diff)));
					else
						self:settext("")
					end
				else
					self:settext("")
				end
			  end
		};
		LoadFont("Common Normal") .. { 
			  InitCommand=cmd(uppercase,true;zoom,0.75;addy,-20;addx,-143;diffuse,color("#000000");maxwidth,130);
			  OnCommand=cmd(diffusealpha,0;smooth,0.2;diffusealpha,0.75);
			  OffCommand=cmd(linear,0.3;diffusealpha,0);
			  CurrentStepsP1ChangedMessageCommand=cmd(queuecommand,"Set"); 
			  PlayerJoinedMessageCommand=cmd(queuecommand,"Set";diffusealpha,0;linear,0.3;diffusealpha,0.75);
			  ChangedLanguageDisplayMessageCommand=cmd(queuecommand,"Set");
			  SetCommand=function(self)
				stepsP1 = GAMESTATE:GetCurrentSteps(PLAYER_1)
				local song = GAMESTATE:GetCurrentSong();
				if song then 
					if stepsP1 ~= nil then
						local st = stepsP1:GetStepsType();
						local diff = stepsP1:GetDifficulty();
						local courseType = GAMESTATE:IsCourseMode() and SongOrCourse:GetCourseType() or nil;
						local cd = GetCustomDifficulty(st, diff, courseType);
						self:settext(THEME:GetString("StepsType",ToEnumShortString(st)));
					else
						self:settext("")
					end
				else
					self:settext("")
				end
			  end
		};
	};
	
-- P2 Difficulty Pane	
t[#t+1] = Def.ActorFrame {
		InitCommand=cmd(visible,GAMESTATE:IsHumanPlayer(PLAYER_2);horizalign,center;x,SCREEN_CENTER_X+210+32;y,SCREEN_CENTER_Y+230+8);
		OnCommand=cmd(zoomy,0.8;diffusealpha,0;smooth,0.4;diffusealpha,1;zoomy,1);
		PlayerJoinedMessageCommand=function(self,param)
			if param.Player == PLAYER_2 then
				(cmd(visible,true;diffusealpha,0;linear,0.3;diffusealpha,1))(self);
			end;
		end;
		OffCommand=cmd(decelerate,0.3;zoomy,0.8;diffusealpha,0);
		LoadActor(THEME:GetPathG("ScreenSelectMusic", "pane background")) .. {
			InitCommand=cmd(zoomx,-1);
			CurrentStepsP2ChangedMessageCommand=cmd(queuecommand,"Set"); 
			PlayerJoinedMessageCommand=cmd(queuecommand,"Set";diffusealpha,0;decelerate,0.3;diffusealpha,1);
			ChangedLanguageDisplayMessageCommand=cmd(queuecommand,"Set");
			SetCommand=function(self)
					stepsP2 = GAMESTATE:GetCurrentSteps(PLAYER_2)
					local song = GAMESTATE:GetCurrentSong();
					if song then 
						if stepsP2 ~= nil then
							local st = stepsP2:GetStepsType();
							local diff = stepsP2:GetDifficulty();
							local courseType = GAMESTATE:IsCourseMode() and SongOrCourse:GetCourseType() or nil;
							local cd = GetCustomDifficulty(st, diff, courseType);
							self:finishtweening():linear(0.2):diffuse(ColorLightTone(CustomDifficultyToColor(cd)));
						else
							self:diffuse(color("#666666"));
						end
					else
						self:diffuse(color("#666666"));
					end
				  end
		};
		LoadFont("StepsDisplay meter") .. { 
			  InitCommand=cmd(zoom,1.25;diffuse,color("#000000");addx,143;addy,13);
			  OnCommand=cmd(diffusealpha,0;smooth,0.2;diffusealpha,0.75);
			  OffCommand=cmd(linear,0.3;diffusealpha,0);
			  CurrentStepsP2ChangedMessageCommand=cmd(queuecommand,"Set"); 
			  PlayerJoinedMessageCommand=cmd(queuecommand,"Set";diffusealpha,0;linear,0.3;diffusealpha,0.75);
			  ChangedLanguageDisplayMessageCommand=cmd(queuecommand,"Set");
			  SetCommand=function(self)
				stepsP2 = GAMESTATE:GetCurrentSteps(PLAYER_2)
				local song = GAMESTATE:GetCurrentSong();
				if song then 
					if stepsP2 ~= nil then
						local st = stepsP2:GetStepsType();
						local diff = stepsP2:GetDifficulty();
						local courseType = GAMESTATE:IsCourseMode() and SongOrCourse:GetCourseType() or nil;
						local cd = GetCustomDifficulty(st, diff, courseType);
						self:settext(stepsP2:GetMeter())
					else
						self:settext("")
					end
				else
					self:settext("")
				end
			  end
		};
		LoadFont("Common Italic Condensed") .. { 
			  InitCommand=cmd(uppercase,true;zoom,1;addy,-40;addx,143;diffuse,color("#000000");maxwidth,115);
			  OnCommand=cmd(diffusealpha,0;smooth,0.2;diffusealpha,0.75);
			  OffCommand=cmd(linear,0.3;diffusealpha,0);
			  CurrentStepsP2ChangedMessageCommand=cmd(queuecommand,"Set"); 
			  PlayerJoinedMessageCommand=cmd(queuecommand,"Set";diffusealpha,0;linear,0.3;diffusealpha,0.75);
			  ChangedLanguageDisplayMessageCommand=cmd(queuecommand,"Set");
			  SetCommand=function(self)
				stepsP2 = GAMESTATE:GetCurrentSteps(PLAYER_2)
				local song = GAMESTATE:GetCurrentSong();
				if song then 
					if stepsP2 ~= nil then
						local st = stepsP2:GetStepsType();
						local diff = stepsP2:GetDifficulty();
						local courseType = GAMESTATE:IsCourseMode() and SongOrCourse:GetCourseType() or nil;
						local cd = GetCustomDifficulty(st, diff, courseType);
						self:settext(THEME:GetString("CustomDifficulty",ToEnumShortString(diff)));
					else
						self:settext("")
					end
				else
					self:settext("")
				end
			  end
		};
		LoadFont("Common Normal") .. { 
			  InitCommand=cmd(uppercase,true;zoom,0.75;addy,-20;addx,143;diffuse,color("#000000");maxwidth,130);
			  OnCommand=cmd(diffusealpha,0;smooth,0.2;diffusealpha,0.75);
			  OffCommand=cmd(linear,0.3;diffusealpha,0);
			  CurrentStepsP2ChangedMessageCommand=cmd(queuecommand,"Set"); 
			  PlayerJoinedMessageCommand=cmd(queuecommand,"Set";diffusealpha,0;linear,0.3;diffusealpha,0.75);
			  ChangedLanguageDisplayMessageCommand=cmd(queuecommand,"Set");
			  SetCommand=function(self)
				stepsP2 = GAMESTATE:GetCurrentSteps(PLAYER_2)
				local song = GAMESTATE:GetCurrentSong();
				if song then 
					if stepsP2 ~= nil then
						local st = stepsP2:GetStepsType();
						local diff = stepsP2:GetDifficulty();
						local courseType = GAMESTATE:IsCourseMode() and SongOrCourse:GetCourseType() or nil;
						local cd = GetCustomDifficulty(st, diff, courseType);
						self:settext(THEME:GetString("StepsType",ToEnumShortString(st)));
					else
						self:settext("")
					end
				else
					self:settext("")
				end
			  end
		};
	};

t[#t+1] = StandardDecorationFromFileOptional("PaneDisplayTextP1","PaneDisplayTextP1");
t[#t+1] = StandardDecorationFromFileOptional("PaneDisplayTextP2","PaneDisplayTextP2");	

t[#t+1] = StandardDecorationFromTable("PercentScore"..ToEnumShortString(PLAYER_1), PercentScore(PLAYER_1));
t[#t+1] = StandardDecorationFromTable("PercentScore"..ToEnumShortString(PLAYER_2), PercentScore(PLAYER_2));


end;

-- BPMDisplay
t[#t+1] = Def.ActorFrame {
    InitCommand=cmd(draworder,126;visible,not GAMESTATE:IsCourseMode());
    OnCommand=cmd(diffusealpha,0;smooth,0.3;diffusealpha,1);
    OffCommand=cmd(linear,0.3;diffusealpha,0);
    LoadFont("Common Condensed") .. {
          InitCommand=cmd(horizalign,right;x,SCREEN_CENTER_X-198+69-66;y,SCREEN_CENTER_Y-78+2;diffuse,color("#512232");horizalign,right;visible,not GAMESTATE:IsCourseMode());
          OnCommand=cmd(queuecommand,"Set");
          ChangedLanguageDisplayMessageCommand=cmd(queuecommand,"Set");
          SetCommand=function(self)
              self:settext("BPM"):diffuse(ColorLightTone(StageToColor(curStage)));
              end;
    };
    StandardDecorationFromFileOptional("BPMDisplay","BPMDisplay");
};


return t;