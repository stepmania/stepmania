local function StepsDisplay(pn)
	local function set(self, player)
		self:SetFromGameState( player );
	end

	local t = Def.StepsDisplay {
		InitCommand=cmd(Load,"StepsDisplay",GAMESTATE:GetPlayerState(pn););
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

local t = LoadFallbackB();

t[#t+1] = Def.ActorFrame { 
	InitCommand=cmd(x,SCREEN_CENTER_X+140;y,SCREEN_CENTER_Y-20);
	OnCommand=cmd(addx,SCREEN_WIDTH*0.6;bounceend,0.5;addx,-SCREEN_WIDTH*0.6);
	OffCommand=cmd(bouncebegin,0.5;addx,SCREEN_WIDTH*0.6);
	LoadActor( "_banner mask" ) .. {
		InitCommand=cmd(y,-74;zwrite,1;z,1;blend,"BlendMode_NoEffect");
	};
	Def.ActorProxy {
		BeginCommand=function(self) local banner = SCREENMAN:GetTopScreen():GetChild('Banner'); self:SetTarget(banner); end;
		InitCommand=cmd(y,-74;);
	};
	LoadActor( "_banner frame" ) .. {
	};
};

t[#t+1] = LoadFont("Common", "normal") .. {
	InitCommand=cmd(x,SCREEN_CENTER_X-160;y,SCREEN_CENTER_Y+94;playcommand,"Set");
	OnCommand=cmd(shadowlength,2;diffusealpha,0;linear,0.5;diffusealpha,1);
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
	
t[#t+1] = Def.ActorFrame {
	InitCommand=cmd(x,SCREEN_CENTER_X-44;y,SCREEN_CENTER_Y-10);
	LoadActor( "wheel cursor glow" ) .. {
		InitCommand=cmd(blend,"BlendMode_Add";diffuseshift);
	};
	LoadActor( "wheel cursor normal" );
};

for pn in ivalues(PlayerNumber) do
	local MetricsName = "OptionsArea" .. PlayerNumberToString(pn);
	local spacing_x = THEME:GetMetric("ModIconRowSelectMusic","SpacingX");
	local spacing_y = THEME:GetMetric("ModIconRowSelectMusic","SpacingY");
	local num = THEME:GetMetric("ModIconRowSelectMusic","NumModIcons");
	t[#t+1] = Def.ActorFrame {
		InitCommand=function(self) self:name(MetricsName); ActorUtil.LoadAllCommandsAndSetXY(self,Var "LoadingScreen"); end;
		Def.ActorFrame {
			InitCommand=cmd(x,(-1-(num-1)/2)*spacing_x;y,(-1-(num-1)/2)*spacing_y;);
			LoadActor( "option icon header" ) .. {
			};
			LoadFont("_terminator two 18px" ) .. {
				InitCommand=cmd(y,-4;settext,PlayerNumberToLocalizedString(pn);diffuse,PlayerColor(pn);shadowlength,0;);
			};
		};
		Def.ModIconRow {
			InitCommand=cmd(Load,"ModIconRowSelectMusic",pn;);
			OnCommand=cmd(zoomy,0;linear,0.5;zoomy,1;);
			OffCommand=cmd(linear,0.5;zoomy,0;);
		};
	};
end
	
t[#t+1] = Def.ActorFrame {
	InitCommand=cmd(x,SCREEN_CENTER_X-120;y,SCREEN_CENTER_Y-170);
	OnCommand=cmd(addx,-SCREEN_WIDTH*0.6;bounceend,0.5;addx,SCREEN_WIDTH*0.6);
	OffCommand=cmd(bouncebegin,0.5;addx,-SCREEN_WIDTH*0.6);
	LoadActor( THEME:GetPathG("MusicSortDisplay","frame") ) .. {
	
	};
	LoadFont( "_sf square head 13px" ) .. {
		InitCommand=cmd(maxwidth,300;playcommand,"Set";x,10;shadowlength,0;diffuse,color("#fbfb57");strokecolor,color("#696800"););
		SetCommand = function(self)
			local so = GAMESTATE:GetSortOrder();
			if so ~= nil then
				self:settext( string.upper( SortOrderToLocalizedString(so)) )
			end;
		end;
		SortOrderChangedMessageCommand=cmd(playcommand,"Set");
	};
};

t[#t+1] = LoadActor( THEME:GetPathG(Var 'LoadingScreen','options message 1x2') ) .. {
	InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;pause);
	OnCommand=cmd(hidden,1;draworder,111);
	ShowPressStartForOptionsCommand=cmd(hidden,0;setstate,0;
		faderight,.3;fadeleft,.3;cropleft,-0.3;cropright,1.3;linear,0.4;cropright,-0.3);
	ShowEnteringOptionsCommand=cmd(finishtweening;setstate,1;sleep,0.25;playcommand,"HidePressStartForOptions");
	HidePressStartForOptionsCommand=cmd(linear,0.4;cropleft,1.3);
};

for pn in ivalues(PlayerNumber) do
	local MetricsName = "PaneDisplay" .. PlayerNumberToString(pn);
	t[#t+1] = Def.PaneDisplay {
		MetricsGroup="PaneDisplay";
		PlayerNumber=pn;
		InitCommand=function(self) self:player(pn); self:playcommand("Set"); self:name(MetricsName); ActorUtil.LoadAllCommandsAndSetXY(self,Var "LoadingScreen"); end;
		SetCommand=function(self) self:SetFromGameState() end;
		CurrentStepsP1ChangedMessageCommand=cmd(playcommand,"Set");
		CurrentTrailP1ChangedMessageCommand=cmd(playcommand,"Set");
		SortOrderChangedMessageCommand=cmd(playcommand,"Set");
	};
end;

t[#t+1] = Def.BPMDisplay {
	File=THEME:GetPathF("BPMDisplay", "bpm");
	Name="BPMDisplay";
	InitCommand=cmd(horizalign,right;x,SCREEN_CENTER_X+294;y,SCREEN_CENTER_Y-9;zoomx,0.8;shadowlengthx,0;shadowlengthy,2;shadowcolor,color("#000000"););
	OnCommand=cmd(stoptweening;addx,SCREEN_WIDTH*0.6;bounceend,0.5;addx,-SCREEN_WIDTH*0.6);
	OffCommand=cmd(bouncebegin,0.5;addx,SCREEN_WIDTH*0.6);
	SetCommand=function(self) self:SetFromGameState() end;
	CurrentSongChangedMessageCommand=cmd(playcommand,"Set");
	CurrentCourseChangedMessageCommand=cmd(playcommand,"Set");
};
t[#t+1] = LoadActor( "_bpm label" ) .. {
	InitCommand=cmd(horizalign,left;x,SCREEN_CENTER_X+280;y,SCREEN_CENTER_Y-10);
	OnCommand=cmd(addx,SCREEN_WIDTH*0.6;bounceend,0.5;addx,-SCREEN_WIDTH*0.6);
	OffCommand=cmd(bouncebegin,0.5;addx,SCREEN_WIDTH*0.6);
};

t[#t+1] = LoadActor( "temp bpm meter" ) .. {
	InitCommand=cmd(x,SCREEN_CENTER_X+230;y,SCREEN_CENTER_Y-22);
	OnCommand=cmd(addx,SCREEN_WIDTH*0.6;bounceend,0.5;addx,-SCREEN_WIDTH*0.6);
	OffCommand=cmd(bouncebegin,0.5;addx,SCREEN_WIDTH*0.6);
};

t[#t+1] = Def.ActorFrame {	
	InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y);
	OnCommand=cmd(addx,SCREEN_WIDTH*0.6;bounceend,0.5;addx,-SCREEN_WIDTH*0.6);
	OffCommand=cmd(bouncebegin,0.5;addx,SCREEN_WIDTH*0.6);
	
	BeginCommand=cmd(visible,false);

	LoadActor( "bpm meter" ) .. {
			InitCommand=cmd(setstate,0;pause)
	};

	Def.Quad {
		InitCommand=cmd(diffuse,color("#FFFFFF");setsize,120,16;horizalign,right;addx,60);
		BeginCommand=cmd(zwrite,1;z,1;blend,"BlendMode_NoEffect");
		UpdateCommand=function(self)
			local function CalcZoomX(fBpm)
				local fWidth;
				if fBpm > 300 then
					fWidth=120;
				elseif fBpm < 60 then
					fWidth=0;
				else
					fWidth=scale(fBpm,60,300,0,120);
				end;
				local fSpacing=12;
				local fWidth2=math.floor((fWidth + fSpacing/2)/fSpacing)*fSpacing;
				return fWidth2/120;
			end;
		end;
	};
	LoadActor( "bpm meter" ) .. {
		InitCommand=cmd(setstate,1;pause;glowshift;effectcolor1,color("1,1,1,0");effectcolor2,color("1,1,1,.1");effectclock,"bgm");
		BeginCommand=cmd(ztest,1);
	};
	
	
};
t[#t+1] = LoadActor( "stop icon" ) .. {
	InitCommand=cmd(x,SCREEN_CENTER_X+296;y,SCREEN_CENTER_Y-4);
	OnCommand=cmd(addx,SCREEN_WIDTH*0.6;bounceend,0.5;addx,-SCREEN_WIDTH*0.6);
	OffCommand=cmd(bouncebegin,0.5;addx,SCREEN_WIDTH*0.6);
	SetCommand=function(self) 
			local b = false;
			local song = GAMESTATE:GetCurrentSong();
			if song then
				b = song:GetTimingData():HasStops(); 
			end;
			self:visible( b );
		end;
	CurrentSongChangedMessageCommand=cmd(playcommand,"Set");
	CurrentCourseChangedMessageCommand=cmd(playcommand,"Set");
};
	
t[#t+1] = LoadFont("_regra Bold 16px") .. {
	InitCommand=cmd(horizalign,left;x,SCREEN_CENTER_X-14;y,SCREEN_CENTER_Y-24;settext,"xxxx";shadowlengthx,0;shadowlengthy,2;shadowcolor,color("#000000");maxwidth,360);
	SetCommand=function(self) 
			local s = "---";
			local song = GAMESTATE:GetCurrentSong(); 
			if song then
				local s2 = song:GetDisplayArtist(); 
				if s2 ~= "" then 
					s = "/" .. s2; 
				end;
			end; 
			self:settext( s );
		end;
	CurrentSongChangedMessageCommand=cmd(playcommand,"Set");
	CurrentCourseChangedMessageCommand=cmd(playcommand,"Set");
	DisplayLanguageChangedMessageCommand=cmd(playcommand,"Set");
};
t[#t+1] = LoadFont("_regra Bold 16px") .. {
	InitCommand=cmd(horizalign,right;x,SCREEN_CENTER_X+224;y,SCREEN_CENTER_Y-6;settext,"xxxx";shadowlengthx,0;shadowlengthy,2;shadowcolor,color("#000000"););
	SetCommand=function(self) 
			local s = "---";
			local song = GAMESTATE:GetCurrentSong(); 
			if song then
				local s2 = song:GetGenre(); 
				if s2 ~= "" then 
						s = "_" .. s2; 
				end;
			end; 
			self:settext( s );
		end;
	CurrentSongChangedMessageCommand=cmd(playcommand,"Set");
	CurrentCourseChangedMessageCommand=cmd(playcommand,"Set");
};

t[#t+1] = Def.ActorFrame {
	InitCommand=cmd(x,SCREEN_CENTER_X+26;y,SCREEN_CENTER_Y-5;);
	LoadActor("star full") .. { InitCommand=cmd(x,16*-2); };
	LoadActor("star full") .. { InitCommand=cmd(x,16*-1); };
	LoadActor("star full") .. { InitCommand=cmd(x,16*0); };
	LoadActor("star empty") .. { InitCommand=cmd(x,16*1); };
	LoadActor("star empty") .. { InitCommand=cmd(x,16*2); };
	CurrentSongChangedMessageCommand=cmd(playcommand,"Set");
	CurrentCourseChangedMessageCommand=cmd(playcommand,"Set");
};

t[#t+1] = Def.CourseContentsList {
	MaxSongs = 5;

	InitCommand=cmd(x,SCREEN_CENTER_X+160;y,SCREEN_CENTER_Y+91);
	OnCommand=cmd(zoomy,0;bounceend,0.3;zoom,1);
	OffCommand=cmd(zoomy,1;bouncebegin,0.3;zoomy,0);
	ShowCommand=cmd(bouncebegin,0.3;zoomy,1);
	HideCommand=cmd(linear,0.3;zoomy,0);
	SetCommand=function(self)
		self:SetFromGameState();
		self:setsecondsperitem(0.7);
		self:SetSecondsPauseBetweenItems(0.7);
		self:scrollwithpadding(0, 0);
	end;
	CurrentTrailP1ChangedMessageCommand=cmd(playcommand,"Set");
	CurrentTrailP2ChangedMessageCommand=cmd(playcommand,"Set");

	Display = Def.ActorFrame { 
		InitCommand=cmd(setsize,270,44);

		LoadActor("_CourseEntryDisplay bar");

		Def.TextBanner {
			InitCommand=cmd(Load,"TextBanner";SetFromString,"", "", "", "", "", "");
			SetSongCommand=function(self, params)
				if params.Song then
					self:SetFromSong( params.Song );
					self:diffuse( SONGMAN:GetSongColor(params.Song) );
				else
					self:SetFromString( "??????????", "??????????", "", "", "", "" );
					self:diffuse( color("#FFFFFF") );
				end
			end;
		};

		LoadFont("CourseEntryDisplay","number") .. {
			OnCommand=cmd(x,-118;shadowlength,0);
			SetSongCommand=function(self, params) self:settext(string.format("%i", params.Number)); end;
		};

		LoadFont("Common","normal") .. {
			OnCommand=cmd(x,SCREEN_CENTER_X-200;y,-8;zoom,0.7;shadowlength,0);
			DifficultyChangedCommand=function(self, params)
				if params.PlayerNumber ~= GAMESTATE:GetMasterPlayerNumber() then return end
				self:settext( params.Meter );
				self:diffuse( CourseDifficutlyToColor(params.Difficulty) );
			end;
		};

		LoadFont("Common","normal") .. {
			OnCommand=cmd(x,SCREEN_CENTER_X-192;y,SCREEN_CENTER_Y-230;horizalign,right;shadowlength,0);
			SetSongCommand=function(self, params) self:settext(params.Modifiers); end;
		};

		LoadFont("CourseEntryDisplay","difficulty") .. {
			OnCommand=cmd(x,SCREEN_CENTER_X-222;y,-8;shadowlength,0;settext,"1");
			DifficultyChangedCommand=function(self, params)
				if params.PlayerNumber ~= GAMESTATE:GetMasterPlayerNumber() then return end
				self:diffuse( CourseDifficutlyToColor(params.Difficulty) );
			end;
		};
	};
};

for pn in ivalues(PlayerNumber) do
	local MetricsName = "StepsDisplay" .. PlayerNumberToString(pn);
	t[#t+1] = StepsDisplay(pn) .. {
		InitCommand=function(self) self:player(pn); self:name(MetricsName); ActorUtil.LoadAllCommandsAndSetXY(self,Var "LoadingScreen"); end;
	};
end


t[#t+1] = Def.ActorFrame{
	InitCommand=cmd(x,SCREEN_CENTER_X-190;y,SCREEN_CENTER_Y-32;diffusealpha,0);
	ShowCommand=cmd(stoptweening;linear,0.2;diffusealpha,1);
	HideCommand=cmd(stoptweening;linear,0.2;diffusealpha,0);
	
	LoadActor("long balloon");
	LoadFont("_terminator two 30px")..{
		Name="SongLength";
		InitCommand=cmd(x,-56;y,-10;shadowlength,0;diffuse,color("#E2E2E2");diffusebottomedge,color("#CECECE");strokecolor,color("#00000000"));
		SetCommand=function(self)
			local Song = GAMESTATE:GetCurrentSong();
			if Song then
				local time = Song:MusicLengthSeconds();
				time = SecondsToMSSMsMs(time);
				time = string.sub(time, 0, string.len(time)-3);
				self:settext( time );
			else
				self:settext( "" );
			end
		end;
	};
	LoadFont("_venacti bold 24px")..{
		Name="NumStages";
		InitCommand=cmd(x,30;y,-6;shadowlengthx,0;shadowlengthy,1;skewx,0;zoom,0.7;diffusebottomedge,color("#068EE1FF");strokecolor,color("#FF000000"));
		SetCommand=function(self)
			local Song = GAMESTATE:GetCurrentSong();
			if Song then
				-- xxx: use localized text
				local postfix = " STAGES";
				local numStages = GAMESTATE:GetNumStagesForCurrentSongAndStepsOrCourse();
				--[[
				if Song:IsLong() then self:settext("2"..postfix);
				elseif Song:IsMarathon() then self:settext("3"..postfix);
				end;
                ]]
				self:settext( numStages..postfix );
			else
				self:settext( "" );
			end
		end;
	};
	
	SetCommand=function(self)
		local Song = GAMESTATE:GetCurrentSong();
		self:playcommandonchildren( (Song and (Song:IsLong() or Song:IsMarathon())) and "Show" or "Hide" );
	end;
	CurrentSongChangedMessageCommand=cmd(playcommand,"Set");
};

t[#t+1] = LoadFont("common normal") .. {
	InitCommand=cmd(x,SCREEN_CENTER_X+262;y,SCREEN_CENTER_Y);
	OnCommand=cmd(shadowlength,0;addx,-SCREEN_WIDTH;bounceend,0.5;addx,SCREEN_WIDTH);
	OffCommand=cmd(bouncebegin,0.5;addx,-SCREEN_WIDTH);

	SetCommand=function(self)
		local Course = GAMESTATE:GetCurrentCourse()
		if not Course then
			self:visible(false)
			return
		end

		self:visible(true)
		self:settext( Course:GetEstimatedNumStages() );
	end;

	CurrentCourseChangedMessageCommand=cmd(playcommand,"Set");
};


if not GAMESTATE:IsCourseMode() then
	t[#t+1] = Def.DifficultyList {
		Name="DifficultyList";
		InitCommand=cmd(x,SCREEN_CENTER_X+166;y,SCREEN_CENTER_Y+20);
		CursorP1 = Def.ActorFrame {
			BeginCommand=cmd(visible,true);
			StepsSelectedMessageCommand=function( self, param ) 
				if param.Player ~= "PlayerNumber_P1" then return end;
				self:visible(false);
			end;
			children={
				LoadActor( "DifficultyList highlight" ) .. {
					InitCommand=cmd(addx,-10;diffusealpha,0.3);
					BeginCommand=cmd(player,"PlayerNumber_P1");
					OnCommand=cmd(playcommand,"UpdateAlpha");
					CurrentStepsP1ChangedMessageCommand=cmd(playcommand,"UpdateAlpha");
					CurrentStepsP2ChangedMessageCommand=cmd(playcommand,"UpdateAlpha");
					UpdateAlphaCommand=function(self)
						local s1 = GAMESTATE:GetCurrentSteps(PLAYER_1);
						local s2 = GAMESTATE:GetCurrentSteps(PLAYER_2);
						self:stoptweening();
						if not s1 or not s2 or s1:GetDifficulty() == s2:GetDifficulty() then
							self:linear(.08);
							self:diffusealpha(0.15);
						else
							self:linear(.08); --has no effect if alpha is already .3
							self:diffusealpha(0.3);
						end;
					end;
					PlayerJoinedMessageCommand=function(self,param )
						if param.Player ~= "PlayerNumber_P1" then return end;
						self:visible( true );
					end;
				};
				Def.ActorFrame {
					InitCommand=cmd(x,-150;bounce;effectmagnitude,-12,0,0;effectperiod,1.0;effectoffset,0.0;effectclock,"bgm");
					children={
						LoadActor( "DifficultyList cursor p1" ) .. {
							BeginCommand=cmd(player,"PlayerNumber_P1";);
							PlayerJoinedMessageCommand=function(self,param )
								if param.Player ~= "PlayerNumber_P1" then return end;
								self:visible( true );
							end;
						};
						LoadFont( "_terminator two 18px" ) .. {
							InitCommand=cmd(x,-4;y,-3;settext,"P1";diffuse,PlayerColor("PlayerNumber_P1");shadowlength,0;);
							BeginCommand=cmd(player,"PlayerNumber_P1";);
							PlayerJoinedMessageCommand=function(self,param )
								if param.Player ~= "PlayerNumber_P1" then return end;
								self:visible( true );
							end;
						};
					}
				};
			}
		};
		CursorP2 = Def.ActorFrame {
			BeginCommand=cmd(visible,true);
			StepsSelectedMessageCommand=function( self, param ) 
				if param.Player ~= "PlayerNumber_P2" then return end;
				self:visible(false);
			end;
			children={
				LoadActor( "DifficultyList highlight" ) .. {
					InitCommand=cmd(addx,-10;zoomx,-1;diffusealpha,0.3);
					BeginCommand=cmd(player,"PlayerNumber_P2");
					OnCommand=cmd(playcommand,"UpdateAlpha");
					CurrentStepsP1ChangedMessageCommand=cmd(playcommand,"UpdateAlpha");
					CurrentStepsP2ChangedMessageCommand=cmd(playcommand,"UpdateAlpha");
					UpdateAlphaCommand=function(self)
						local s1 = GAMESTATE:GetCurrentSteps(PLAYER_1);
						local s2 = GAMESTATE:GetCurrentSteps(PLAYER_2);
						self:stoptweening();
						if not s1 or not s2 or s1:GetDifficulty() == s2:GetDifficulty() then
							self:linear(.08);
							self:diffusealpha(0.15);
						else
							self:linear(.08); --has no effect if alpha is already .3
							self:diffusealpha(0.3);
						end;
					end;
					PlayerJoinedMessageCommand=function(self,param )
						if param.Player ~= "PlayerNumber_P2" then return end;
						self:visible( true );
					end;
				};
				Def.ActorFrame {
					InitCommand=cmd(x,130;bounce;effectmagnitude,12,0,0;effectperiod,1.0;effectoffset,0.0;effectclock,"bgm");
					children={
						LoadActor( "DifficultyList cursor p2" ) .. {
							BeginCommand=cmd(player,"PlayerNumber_P2";);
							PlayerJoinedMessageCommand=function(self,param )
								if param.Player ~= "PlayerNumber_P2" then return end;
								self:visible( true );
							end;
						};
						LoadFont( "_terminator two 18px" ) .. {
							InitCommand=cmd(x,4;y,-3;settext,"P2";diffuse,PlayerColor("PlayerNumber_P2");shadowlength,0;);
							BeginCommand=cmd(player,"PlayerNumber_P2";);
							PlayerJoinedMessageCommand=function(self,param )
								if param.Player ~= "PlayerNumber_P2" then return end;
								self:visible( true );
							end;
						};
					}
				};
			}
		};
		CursorP1Frame = Def.Actor{ };
		CursorP2Frame = Def.Actor{ };
	};
end
-- fade out
t[#t+1] = Def.Quad {
		InitCommand=cmd(FullScreen;draworder,110;diffuse,color("#000000");diffusealpha,0);
		OffCommand=cmd(linear,0.3;diffusealpha,1);
};

return t;
