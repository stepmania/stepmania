local function Autogen(pn)
	local function set(self, player)
		if player and player ~= pn then return end
		local Selection = GAMESTATE:GetCurrentSteps(pn) or GAMESTATE:GetCurrentCourse()
		local bIsAutogen = Selection and Selection:IsAutogen();
		self:hidden( bIsAutogen and 0 or 1 );
	end

	local t = LoadActor( "_autogen" ) .. {
		OnCommand=cmd(zoomy,0;linear,0.5;zoomy,1);
		OffCommand=cmd(linear,0.5;zoomy,0);
		CurrentStepsP1ChangedMessageCommand=function(self) set(self, PLAYER_1); end;
		CurrentStepsP2ChangedMessageCommand=function(self) set(self, PLAYER_2); end;
		CurrentCourseChangedMessageCommand=function(self) set(self); end;
	};
	return t;
end

local function DifficultyMeter(pn)
	local function set(self, player)
		self:SetFromGameState( player );
	end

	local t = Def.DifficultyMeter {
		Type="DifficultyMeter";
	};

	if pn == PLAYER_1 then
		t.CurrentStepsP1ChangedMessageCommand=function(self) set(self, pn); end;
		t.CurrentTrailP1ChangedMessageCommand=function(self) set(self, pn); end;
	else
		t.CurrentStepsP2ChangedMessageCommand=function(self) set(self, pn); end;
		t.CurrentTrailP2ChangedMessageCommand=function(self) set(self, pn); end;
	end

	return t;
end

local function DifficultyIcons(pn)
	local function set(self, player)
		if player and player ~= pn then return end
		local Selection = GAMESTATE:GetCurrentSteps(pn) or GAMESTATE:GetCurrentTrail(pn)

		if not Selection then
			self:Unset();
			return
		end
		local dc = Selection:GetDifficulty()
		self:SetFromDifficulty( dc );
	end

	local t = Def.DifficultyIcon {
		File="_difficulty icons 1x6";
		InitCommand=function(self)
			self:player( pn );
			self:SetPlayer( pn );
		end;

		CurrentStepsP1ChangedMessageCommand=function(self) set(self, PLAYER_1); end;
		CurrentStepsP2ChangedMessageCommand=function(self) set(self, PLAYER_2); end;
		CurrentTrailP1ChangedMessageCommand=function(self) set(self, PLAYER_1); end;
		CurrentTrailP2ChangedMessageCommand=function(self) set(self, PLAYER_2); end;
	};
	return t;
end

local function Radar()
	local function set(self,player)
		local Selection = GAMESTATE:GetCurrentSteps(player) or GAMESTATE:GetCurrentTrail(player)
		if not Selection then
			self:SetEmpty( player );
			return
		end
		self:SetFromRadarValues( player, Selection:GetRadarValues(player) );

	end

	local t = Def.GrooveRadar {
		OnCommand=cmd(tweenonscreen);
		OffCommand=cmd(tweenoffscreen);

		CurrentStepsP1ChangedMessageCommand=function(self) set(self, PLAYER_1); end;
		CurrentStepsP2ChangedMessageCommand=function(self) set(self, PLAYER_2); end;
		CurrentTrailP1ChangedMessageCommand=function(self) set(self, PLAYER_1); end;
		CurrentTrailP2ChangedMessageCommand=function(self) set(self, PLAYER_2); end;
	};

	return t;
end

local children = 
{
	LoadActor( "_banner mask" ) .. {
		InitCommand=cmd(y,SCREEN_CENTER_Y-84;blend,"BlendMode_NoEffect";zwrite,1);
		OnCommand=cmd(x,SCREEN_CENTER_X-180-SCREEN_WIDTH*0.6;bounceend,0.5;addx,SCREEN_WIDTH*0.6);
		OffCommand=cmd(bouncebegin,0.5;addx,-SCREEN_WIDTH*0.6);
	};

	Def.ActorProxy {
		BeginCommand=function(self) local banner = SCREENMAN:GetTopScreen():GetChild('Banner'); self:SetTarget(banner); end;
		InitCommand=cmd(y,SCREEN_CENTER_Y-84);
		OnCommand=cmd(x,SCREEN_CENTER_X-180-SCREEN_WIDTH*0.6;bounceend,0.5;addx,SCREEN_WIDTH*0.6);
		OffCommand=cmd(bouncebegin,0.5;addx,-SCREEN_WIDTH*0.6);
	};

	LoadActor( "_banner frame" ) .. {
		InitCommand=cmd(y,SCREEN_CENTER_Y-97);
		OnCommand=cmd(x,SCREEN_CENTER_X-180-SCREEN_WIDTH*0.6;bounceend,0.5;addx,SCREEN_WIDTH*0.6);
		OffCommand=cmd(bouncebegin,0.5;addx,-SCREEN_WIDTH*0.6);
	};

	Radar() .. {
		BeginCommand=cmd(x,SCREEN_CENTER_X-156;y,SCREEN_CENTER_Y+105);
		Condition=GAMESTATE:IsCourseMode() == false;
	};

	Def.BitmapText {
		Font="Common normal";

		InitCommand=cmd(x,SCREEN_CENTER_X-160;y,SCREEN_CENTER_Y+94;playcommand,"Set");
		OnCommand=cmd(zoom,0.5;shadowlength,2;diffusealpha,0;linear,0.5;diffusealpha,1);
		OffCommand=cmd(linear,0.5;diffusealpha,0);

		SetCommand=function(self)
			local sText = GAMESTATE:GetSongOptionsString()
			sText = string.gsub(sText, ", ", "\n")
			self:settext( sText )
			if GAMESTATE:IsAnExtraStage() then
				self:diffuseblink()
			end
		end;
		SongOptionsChangedMessageCommand=cmd(playcommand,"Set");
	};
	Def.OptionIconRow {
		Condition=GAMESTATE:IsHumanPlayer(PLAYER_1);
		InitCommand=cmd(x,SCREEN_CENTER_X-300;y,SCREEN_CENTER_Y-182;set,PLAYER_1);
		OnCommand=cmd(zoomy,0;linear,0.5;zoomy,1);
		OffCommand=cmd(linear,0.5;zoomy,0);
		PlayerOptionsChangedP1MessageCommand=cmd(set,PLAYER_1);
	};

	Def.OptionIconRow {
		Condition=GAMESTATE:IsHumanPlayer(PLAYER_2);
		InitCommand=cmd(x,SCREEN_CENTER_X-300;y,SCREEN_CENTER_Y-164;set,PLAYER_2);
		OnCommand=cmd(zoomy,0;linear,0.5;zoomy,1);
		OffCommand=cmd(linear,0.5;zoomy,0);
		PlayerOptionsChangedP2MessageCommand=cmd(set,PLAYER_2);
	};

	Def.MusicSortDisplay {
		Condition=not GAMESTATE:IsCourseMode();
		InitCommand=cmd(x,SCREEN_CENTER_X-65-SCREEN_WIDTH*0.6;y,SCREEN_CENTER_Y-135);
		OnCommand=cmd(bounceend,0.5;addx,SCREEN_WIDTH*0.6);
		OffCommand=cmd(bouncebegin,0.5;addx,-SCREEN_WIDTH*0.6);
	};
	-- Do the fade out here, because we want the options message to
	-- appear over it.
	Def.Quad {
		InitCommand=cmd(stretchto,SCREEN_LEFT,SCREEN_TOP,SCREEN_RIGHT,SCREEN_BOTTOM;diffuse,color("#000000"));
		OnCommand=cmd(diffusealpha,0);
		OffCommand=cmd(linear,0.3;diffusealpha,1);
	};

	LoadActor( THEME:GetPathG(Var 'LoadingScreen','options message 1x2') ) .. {
		InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;pause);
		OnCommand=cmd(hidden,1);
		ShowPressStartForOptionsCommand=cmd(hidden,0;setstate,0;
			faderight,.3;fadeleft,.3;cropleft,-0.3;cropright,1.3;linear,0.4;cropright,-0.3);
		ShowEnteringOptionsCommand=cmd(finishtweening;setstate,1;sleep,0.25;playcommand,"HidePressStartForOptions");
		HidePressStartForOptionsCommand=cmd(linear,0.4;cropleft,1.3);
	};
	
	Def.PaneDisplay {
		MetricsGroup="PaneDisplay";
		PlayerNumber=PLAYER_1;
		InitCommand=cmd(player,PLAYER_1;playcommand,"Set";x,SCREEN_CENTER_X-257;y,SCREEN_CENTER_Y+36);
		
		SetCommand=function(self) self:SetFromGameState() end;
		CurrentStepsP1ChangedMessageCommand=cmd(playcommand,"Set");
		CurrentTrailP1ChangedMessageCommand=cmd(playcommand,"Set");
		SortOrderChangedMessageCommand=cmd(playcommand,"Set");
	};

	Def.PaneDisplay {
		Type="PaneDisplay";
		MetricsGroup="PaneDisplay";
		PlayerNumber=PLAYER_2;
		InitCommand=cmd(player,PLAYER_2;playcommand,"Set";x,SCREEN_CENTER_X-47;y,SCREEN_CENTER_Y+36);
		
		SetCommand=function(self) self:SetFromGameState() end;
		CurrentStepsP2ChangedMessageCommand=cmd(playcommand,"Set");
		CurrentTrailP2ChangedMessageCommand=cmd(playcommand,"Set");
		SortOrderChangedMessageCommand=cmd(playcommand,"Set");
	};

	Def.BPMDisplay {
		Name="BPMDisplay";
		Font="BPMDisplay bpm";
		InitCommand=cmd(horizalign,right;y,SCREEN_CENTER_Y-134;shadowlength,0;);
		OnCommand=cmd(stoptweening;x,SCREEN_CENTER_X-160-SCREEN_WIDTH*0.6;bounceend,0.5;addx,SCREEN_WIDTH*0.6);
		OffCommand=cmd(bouncebegin,0.5;addx,-SCREEN_WIDTH*0.6);
		SetCommand=function(self) self:SetFromGameState() end;
		CurrentSongChangedMessageCommand=cmd(playcommand,"Set");
		CurrentCourseChangedMessageCommand=cmd(playcommand,"Set");
	};
	LoadActor( "_bpm label" ) .. {
		InitCommand=cmd(horizalign,left);
		OnCommand=cmd(x,SCREEN_CENTER_X-160-SCREEN_WIDTH*0.6;y,SCREEN_CENTER_Y-133;bounceend,0.5;addx,SCREEN_WIDTH*0.6);
		OffCommand=cmd(bouncebegin,0.5;addx,-SCREEN_WIDTH*0.6);
	};
	Def.CourseContentsList {
		InitCommand=cmd(x,SCREEN_CENTER_X-160;y,SCREEN_CENTER_Y+96);
		OnCommand=cmd(zoomy,0;bounceend,0.3;zoom,1);
		OffCommand=cmd(zoomy,1;bouncebegin,0.3;zoomy,0);
		ShowCommand=cmd(bouncebegin,0.3;zoomy,1);
		HideCommand=cmd(linear,0.3;zoomy,0);
		SetCommand=function(self) self:SetFromGameState() end;
		CurrentTrailP1ChangedMessageCommand=cmd(playcommand,"Set");
		CurrentTrailP2ChangedMessageCommand=cmd(playcommand,"Set");
	};

	Autogen(PLAYER_1) .. {
		BeginCommand=cmd(x,SCREEN_CENTER_X-290;y,SCREEN_CENTER_Y-15);
	};
	Autogen(PLAYER_2) .. {
		BeginCommand=cmd(x,SCREEN_CENTER_X-68;y,SCREEN_CENTER_Y-15);
	};
	
	DifficultyIcons(PLAYER_1) .. {
		BeginCommand=cmd(x,SCREEN_CENTER_X-290;y,SCREEN_CENTER_Y-15);
		OnCommand=cmd(zoomy,0;linear,0.5;zoomy,1);
		OffCommand=cmd(linear,0.5;zoomy,0);
	};
	DifficultyIcons(PLAYER_2) .. {
		BeginCommand=cmd(x,SCREEN_CENTER_X-68;y,SCREEN_CENTER_Y-15);
		OnCommand=cmd(zoomy,0;linear,0.5;zoomy,1);
		OffCommand=cmd(linear,0.5;zoomy,0);
	};

	DifficultyMeter(PLAYER_1) .. {
		BeginCommand=cmd(player,PLAYER_1;x,SCREEN_CENTER_X-254;y,SCREEN_CENTER_Y-20);
		OnCommand=cmd(zoomy,0;linear,0.5;zoomy,1);
		OffCommand=cmd(linear,0.5;zoomy,0);
	};
	DifficultyMeter(PLAYER_2) .. {
		BeginCommand=cmd(player,PLAYER_2;x,SCREEN_CENTER_X-121;y,SCREEN_CENTER_Y-20);
		OnCommand=cmd(zoomy,0;linear,0.5;zoomy,1);
		OffCommand=cmd(linear,0.5;zoomy,0);
	};

	LoadActor( "_balloon long" ) .. {
		InitCommand=cmd(x,SCREEN_CENTER_X+58;y,SCREEN_CENTER_Y-33;playcommand,"Set";finishtweening);
		OnCommand=cmd(playcommand,"Set");
		OffCommand=cmd(linear,0.2;diffusealpha,0;rotationz,-25);
		ShowCommand=cmd(stoptweening;diffusealpha,0;linear,0.2;diffusealpha,1;rotationz,0);
		HideCommand=cmd(stoptweening;linear,0.2;diffusealpha,0;rotationz,-25);
		SetCommand=function(self)
			local Song = GAMESTATE:GetCurrentSong()
			self:playcommand( (Song and Song:IsLong()) and "Show" or "Hide" );
		end;
		CurrentSongChangedMessageCommand=cmd(playcommand,"Set");
	};
	LoadActor( "_balloon marathon" ) .. {
		InitCommand=cmd(x,SCREEN_CENTER_X+58;y,SCREEN_CENTER_Y-33;playcommand,"Set";finishtweening);
		OnCommand=cmd(playcommand,"Set");
		OffCommand=cmd(linear,0.2;diffusealpha,0;rotationz,-25);
		ShowCommand=cmd(stoptweening;diffusealpha,0;linear,0.2;diffusealpha,1;rotationz,0);
		HideCommand=cmd(stoptweening;linear,0.2;diffusealpha,0;rotationz,-25);
		SetCommand=function(self)
			local Song = GAMESTATE:GetCurrentSong()
			self:playcommand( (Song and Song:IsMarathon()) and "Show" or "Hide" );
		end;
		CurrentSongChangedMessageCommand=cmd(playcommand,"Set");
	};

	Def.BitmapText {
		Font="_numbers2";
		InitCommand=cmd(x,SCREEN_CENTER_X-262;y,SCREEN_CENTER_Y-126);
		OnCommand=cmd(shadowlength,0;addx,-SCREEN_WIDTH;bounceend,0.5;addx,SCREEN_WIDTH);
		OffCommand=cmd(bouncebegin,0.5;addx,-SCREEN_WIDTH);

		SetCommand=function(self)
			local Course = GAMESTATE:GetCurrentCourse()
			if not Course then
				self:hidden(1)
				return
			end

			self:hidden(0)
			self:settext( Course:GetEstimatedNumStages() );
		end;

		CurrentCourseChangedMessageCommand=cmd(playcommand,"Set");
	};
	LoadActor( "difficulties" ) .. {
		InitCommand=cmd(y,SCREEN_CENTER_Y-134);
		OnCommand=cmd(finishtweening;x,SCREEN_CENTER_X-35-SCREEN_WIDTH*0.6;bounceend,0.5;addx,SCREEN_WIDTH*0.6);
		OffCommand=cmd(finishtweening;bouncebegin,0.5;addx,-SCREEN_WIDTH*0.6);
	};
	Def.BitmapText {
		Font="BPMDisplay";
	
		SetCommand=function(self)
			local song = GAMESTATE:GetCurrentSong()
			local course = GAMESTATE:GetCurrentCourse()
			if not song and not course then
				self:hidden(1)
				return
			end
			self:hidden(0)
			local time
			if song then time = song:MusicLengthSeconds() end
			if course then
				local st = GAMESTATE:GetCurrentStyle():GetStepsType()
				time = course:GetTotalSeconds( st )
			end
			if time then
				self:settext( SecondsToMSSMsMs(time) );
			else
				self:settext( "xx:xx.xx" );
			end
		end;

		InitCommand=cmd(x,SCREEN_CENTER_X-156;y,SCREEN_CENTER_Y+190;shadowlength,0);
		OnCommand=cmd(diffusealpha,0;linear,0.5;diffusealpha,1);
		OffCommand=cmd(linear,0.5;diffusealpha,0);

		CurrentSongChangedMessageCommand=cmd(playcommand,"Set");
		CurrentCourseChangedMessageCommand=cmd(playcommand,"Set");
	};
};
return Def.ActorFrame { children=children };
