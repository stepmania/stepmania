local function GraphDisplay( pn )
	local t = Def.ActorFrame {
		Def.GraphDisplay {
			InitCommand=function(self)
				self:Load("GraphDisplay");
			end;
			BeginCommand=function(self)
				local ss = SCREENMAN:GetTopScreen():GetStageStats();
				self:Set( ss, ss:GetPlayerStageStats(pn) );
				self:player( pn );
			end
		};
	};
	return t;
end

local function ComboGraph( pn )
	local t = Def.ActorFrame {
		Def.ComboGraph {
			InitCommand=function(self)
				self:Load("ComboGraph");
			end;
			BeginCommand=function(self)
				local ss = SCREENMAN:GetTopScreen():GetStageStats();
				self:Set( ss, ss:GetPlayerStageStats(pn) );
				self:player( pn );
			end
		};
	};
	return t;
end

local function PercentScore( pn )
	local t = LoadFont("Common normal")..{
		InitCommand=function(self)
			self:zoom(0.625);
			self:shadowlength(1);
			self:player(pn);
		end;
		BeginCommand=function(self)
			self:playcommand("Set");
		end;
		SetCommand=function(self)
			-- todo: color by difficulty
			local SongOrCourse, StepsOrTrail;
			if GAMESTATE:IsCourseMode() then
				SongOrCourse = GAMESTATE:GetCurrentCourse()
				StepsOrTrail = GAMESTATE:GetCurrentTrail(pn)
			else
				SongOrCourse = GAMESTATE:GetCurrentSong()
				StepsOrTrail = GAMESTATE:GetCurrentSteps(pn)
			end;
			if SongOrCourse and StepsOrTrail then
				local st = StepsOrTrail:GetStepsType();
				local diff = StepsOrTrail:GetDifficulty();
				local courseType = GAMESTATE:IsCourseMode() and SongOrCourse:GetCourseType() or nil;
				local cd = GetCustomDifficulty(st, diff, courseType);
				self:diffuse(CustomDifficultyToColor(cd));
				self:shadowcolor(CustomDifficultyToDarkColor(cd));
			end

			local pss = STATSMAN:GetPlayedStageStats(1):GetPlayerStageStats(pn);
			if pss then
				local pct = pss:GetPercentDancePoints();
				if pct == 1 then
					self:settext("100%");
				else
					self:settext(FormatPercentScore(pct));
				end;
			end;
		end;
	};
	return t;
end

local t = LoadFallbackB();

t[#t+1] = StandardDecorationFromFileOptional("StageDisplay","StageDisplay");

if ShowStandardDecoration("GraphDisplay") then
	for pn in ivalues(GAMESTATE:GetHumanPlayers()) do
		t[#t+1] = StandardDecorationFromTable( "GraphDisplay" .. ToEnumShortString(pn), GraphDisplay(pn) );
	end
end

if ShowStandardDecoration("ComboGraph") then
	for pn in ivalues(GAMESTATE:GetHumanPlayers()) do
		t[#t+1] = StandardDecorationFromTable( "ComboGraph" .. ToEnumShortString(pn), ComboGraph(pn) );
	end
end

if ShowStandardDecoration("StepsDisplay") then
	for pn in ivalues(PlayerNumber) do
		local t2 = Def.StepsDisplay {
			InitCommand=function(self)
				self:Load("StepsDisplayEvaluation",pn);
				self:SetFromGameState(pn);
			end;
			UpdateNetEvalStatsMessageCommand=function(self,param)
				if GAMESTATE:IsPlayerEnabled(pn) then
					self:SetFromSteps(param.Steps)
				end;
			end;
		};
		t[#t+1] = StandardDecorationFromTable( "StepsDisplay" .. ToEnumShortString(pn), t2 );
		t[#t+1] = StandardDecorationFromTable( "PercentScore" .. ToEnumShortString(pn), PercentScore(pn) );
	end
end

for pn in ivalues(PlayerNumber) do
	local MetricsName = "MachineRecord" .. PlayerNumberToString(pn);
	t[#t+1] = LoadActor( THEME:GetPathG(Var "LoadingScreen", "MachineRecord"), pn ) .. {
		InitCommand=function(self) 
			self:player(pn); 
			self:name(MetricsName); 
			ActorUtil.LoadAllCommandsAndSetXY(self,Var "LoadingScreen"); 
		end;
	};
end

for pn in ivalues(PlayerNumber) do
	local MetricsName = "PersonalRecord" .. PlayerNumberToString(pn);
	t[#t+1] = LoadActor( THEME:GetPathG(Var "LoadingScreen", "PersonalRecord"), pn ) .. {
		InitCommand=function(self) 
			self:player(pn); 
			self:name(MetricsName); 
			ActorUtil.LoadAllCommandsAndSetXY(self,Var "LoadingScreen"); 
		end;
	};
end

for pn in ivalues(PlayerNumber) do
	local MetricsName = "StageAward" .. PlayerNumberToString(pn);
	t[#t+1] = LoadActor( THEME:GetPathG(Var "LoadingScreen", "StageAward"), pn ) .. {
		InitCommand=function(self) 
			self:player(pn); 
			self:name(MetricsName); 
			ActorUtil.LoadAllCommandsAndSetXY(self,Var "LoadingScreen"); 
		end;
		BeginCommand=function(self)
			self:playcommand("Set");
		end;
		SetCommand=function(self)
			local tStats = THEME:GetMetric(Var "LoadingScreen", "Summary") and STATSMAN:GetAccumPlayedStageStats() or STATSMAN:GetCurStageStats();
			tStats = tStats:GetPlayerStageStats(pn);
			if tStats:GetStageAward() and not tStats:GetFailed() then
				self:settext( THEME:GetString( "StageAward", ToEnumShortString( tStats:GetStageAward() ) ) );
			else
				self:settext( "" );
			end
		end;
	};
end

for pn in ivalues(PlayerNumber) do
	local MetricsName = "PeakComboAward" .. PlayerNumberToString(pn);
	t[#t+1] = LoadActor( THEME:GetPathG(Var "LoadingScreen", "PeakComboAward"), pn ) .. {
		InitCommand=function(self) 
			self:player(pn); 
			self:name(MetricsName); 
			ActorUtil.LoadAllCommandsAndSetXY(self,Var "LoadingScreen"); 
		end;
		BeginCommand=function(self)
			self:playcommand("Set");
		end;
		SetCommand=function(self)
			local tStats = THEME:GetMetric(Var "LoadingScreen", "Summary") and STATSMAN:GetAccumPlayedStageStats() or STATSMAN:GetCurStageStats();
			tStats = tStats:GetPlayerStageStats(pn);
			if tStats:GetPeakComboAward() then
				self:settext( THEME:GetString( "PeakComboAward", ToEnumShortString( tStats:GetPeakComboAward() ) ) );
			else
				self:settext( "" );
			end
		end;
	};
end

t[#t+1] = StandardDecorationFromFileOptional("SongInformation","SongInformation") .. {
	BeginCommand=function(self)
		local SongOrCourse;
		if GAMESTATE:GetCurrentSong() then
			SongOrCourse = GAMESTATE:GetCurrentSong();
		elseif GAMESTATE:GetCurrentCourse() then
			SongOrCourse = GAMESTATE:GetCurrentCourse();
		else
			return
		end
		
		if SongOrCourse:HasBanner() then
			self:visible(false);
		else
			self:visible(true);
		end
	end;
	SetCommand=function(self)
		local c = self:GetChildren();
		local SongOrCourse;
		if GAMESTATE:GetCurrentSong() then
			SongOrCourse = GAMESTATE:GetCurrentSong();

			c.TextTitle:settext( SongOrCourse:GetDisplayMainTitle() or nil );
			c.TextSubtitle:settext( SongOrCourse:GetDisplaySubTitle() or nil );
			c.TextArtist:settext( SongOrCourse:GetDisplayArtist() or nil );

			if SongOrCourse:GetDisplaySubTitle() == "" then
				c.TextTitle:visible(true);
				c.TextTitle:y(-16.5/2);
				c.TextSubtitle:visible(false);
				c.TextSubtitle:y(0);
				c.TextArtist:visible(true);
				c.TextArtist:y(18/2);
			else
				c.TextTitle:visible(true);
				c.TextTitle:y(-16.5);
				c.TextSubtitle:visible(true);
				c.TextSubtitle:y(0);
				c.TextArtist:visible(true);
				c.TextArtist:y(18);
			end
-- 			self:playcommand("Tick");
		elseif GAMESTATE:GetCurrentCourse() then
			SongOrCourse = GAMESTATE:GetCurrentCourse();
			
			c.TextTitle:settext( SongOrCourse:GetDisplayMainTitle() or nil );
			c.TextSubtitle:settext( SongOrCourse:GetDisplaySubTitle() or nil );
			c.TextArtist:settext( SongOrCourse:GetDisplayArtist() or nil );
			
-- 			self:playcommand("Tick");
		else
			SongOrCourse = nil;
			
			c.TextTitle:settext("");
			c.TextSubtitle:settext("");
			c.TextArtist:settext("");
			
			self:playcommand("Hide")
		end
	end;
	CurrentSongChangedMessageCommand=function(self)
		self:playcommand("Set");
	end;
	CurrentCourseChangedMessageCommand=function(self)
		self:playcommand("Set");
	end;
	DisplayLanguageChangedMessageCommand=function(self)
		self:playcommand("Set");
	end;
};
t[#t+1] = StandardDecorationFromFileOptional("LifeDifficulty","LifeDifficulty");
t[#t+1] = StandardDecorationFromFileOptional("TimingDifficulty","TimingDifficulty");
t[#t+1] = StandardDecorationFromFileOptional("GameType","GameType");
t[#t+1] = Def.ActorFrame {
	Condition=GAMESTATE:HasEarnedExtraStage() and GAMESTATE:IsExtraStage() and not GAMESTATE:IsExtraStage2();
	InitCommand=function(self)
		self:draworder(105);
	end;
	LoadActor( THEME:GetPathS("ScreenEvaluation","try Extra1" ) ) .. {
		Condition=THEME:GetMetric( Var "LoadingScreen","Summary" ) == false;
		OnCommand=function(self)
			self:play();
		end;
	};
	LoadActor( THEME:GetPathG("ScreenStageInformation","Stage extra1" ) ) .. {
		Condition=THEME:GetMetric( Var "LoadingScreen","Summary" ) == false;
		InitCommand=function(self)
			self:Center();
		end;
		OnCommand=function(self)
			self:diffusealpha(0);
			self:zoom(0.85);
			self:bounceend(1);
			self:zoom(1);
			self:diffusealpha(1);
			self:sleep(0);
			self:glow(Color("White"));
			self:decelerate(1);
			self:glow(Color("Invisible"));
			self:smooth(0.35);
			self:zoom(0.25);
			self:y(SCREEN_BOTTOM-72);
		end;
	};
};
t[#t+1] = Def.ActorFrame {
	Condition=GAMESTATE:HasEarnedExtraStage() and not GAMESTATE:IsExtraStage() and GAMESTATE:IsExtraStage2();
	InitCommand=function(self)
		self:draworder(105);
	end;
	LoadActor( THEME:GetPathS("ScreenEvaluation","try Extra2" ) ) .. {
		Condition=THEME:GetMetric( Var "LoadingScreen","Summary" ) == false;
		OnCommand=function(self)
			self:play();
		end;
	};
	LoadActor( THEME:GetPathG("ScreenStageInformation","Stage extra2" ) ) .. {
		Condition=THEME:GetMetric( Var "LoadingScreen","Summary" ) == false;
		InitCommand=function(self)
			self:Center();
		end;
		OnCommand=function(self)
			self:diffusealpha(0);
			self:zoom(0.85);
			self:bounceend(1);
			self:zoom(1);
			self:diffusealpha(1);
			self:sleep(0);
			self:glow(Color("White"));
			self:decelerate(1);
			self:glow(Color("Invisible"));
			self:smooth(0.35);
			self:zoom(0.25);
			self:y(SCREEN_BOTTOM-72);
		end;
	};
};
return t