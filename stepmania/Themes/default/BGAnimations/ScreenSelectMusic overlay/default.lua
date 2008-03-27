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

local function DifficultyDisplay(pn)
	local function set(self, player)
		self:SetFromGameState( player );
	end

	local t = Def.DifficultyDisplay {
		InitCommand=cmd(Load,"DifficultyDisplay",GAMESTATE:GetPlayerState(pn););
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


local fallback_screen = THEME:GetMetric(Var 'LoadingScreen','Fallback');
local element = "overlay";
local fallback_path = THEME:GetPathB(fallback_screen,element);
Trace('fallback_path ' .. fallback_path );

local t = Def.ActorFrame {
	LoadActor( fallback_path );
	
	Def.ActorFrame { 
		InitCommand=cmd(x,SCREEN_CENTER_X+140;y,SCREEN_CENTER_Y-10);
		OnCommand=cmd(addx,-SCREEN_WIDTH*0.6;bounceend,0.5;addx,SCREEN_WIDTH*0.6);
		OffCommand=cmd(bouncebegin,0.5;addx,-SCREEN_WIDTH*0.6);
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

	LoadFont("Common", "normal") .. {
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
	
	Def.ActorFrame {
		InitCommand=cmd(x,SCREEN_CENTER_X-56;y,SCREEN_CENTER_Y);
		LoadActor( "wheel cursor glow" ) .. {
			InitCommand=cmd(blend,"BlendMode_Add";);
		};
		LoadActor( "wheel cursor normal" ) .. {
		
		};
	};
		
	Def.ActorFrame {
		InitCommand=cmd(x,SCREEN_CENTER_X+56;y,SCREEN_CENTER_Y+136);
		Def.ActorFrame {
			InitCommand=cmd(x,-1*THEME:GetMetric("OptionIconRow","SpacingX");y,-1*THEME:GetMetric("OptionIconRow","SpacingY"););
			LoadActor( "option icon header" ) .. {
			
			};
			LoadFont("_terminator two 32" ) .. {
				InitCommand=cmd(y,-4;settext,"P1";zoom,0.5;zoom,0.5;diffuse,color("#baa200");shadowlength,0;strokecolor,color("#00000000"););
			};
		};
		Def.OptionIconRow {
			Condition=GAMESTATE:IsHumanPlayer(PLAYER_1);
			InitCommand=cmd(set,PLAYER_1);
			OnCommand=cmd(zoomy,0;linear,0.5;zoomy,1);
			OffCommand=cmd(linear,0.5;zoomy,0);
			PlayerOptionsChangedP1MessageCommand=cmd(set,PLAYER_1);
		};
	};

	Def.ActorFrame {
		InitCommand=cmd(x,SCREEN_CENTER_X+56;y,SCREEN_CENTER_Y+154);
		Def.ActorFrame {
			InitCommand=cmd(x,-1*THEME:GetMetric("OptionIconRow","SpacingX");y,-1*THEME:GetMetric("OptionIconRow","SpacingY"););
			LoadActor( "option icon header" ) .. {
			
			};
			LoadFont("_terminator two 32") .. {
				InitCommand=cmd(y,-4;settext,"P2";zoom,0.5;zoom,0.5;diffuse,color("#83b767");shadowlength,0;strokecolor,color("#00000000"););
			};
		};
		Def.OptionIconRow {
			Condition=GAMESTATE:IsHumanPlayer(PLAYER_2);
			InitCommand=cmd(set,PLAYER_2);
			OnCommand=cmd(zoomy,0;linear,0.5;zoomy,1);
			OffCommand=cmd(linear,0.5;zoomy,0);
			PlayerOptionsChangedP2MessageCommand=cmd(set,PLAYER_2);
		};
	};

	Def.ActorFrame {
		InitCommand=cmd(x,SCREEN_CENTER_X-120;y,SCREEN_CENTER_Y-170);
		OnCommand=cmd(addx,-SCREEN_WIDTH*0.6;bounceend,0.5;addx,SCREEN_WIDTH*0.6);
		OffCommand=cmd(bouncebegin,0.5;addx,-SCREEN_WIDTH*0.6);
		LoadActor( THEME:GetPathG("MusicSortDisplay","frame") ) .. {
		
		};
		LoadFont( "_sf square head bold stroke 28" ) .. {
			InitCommand=cmd(zoom,0.5;maxwidth,300;playcommand,"Set";x,10;shadowlength,0;diffuse,color("#696800"););
			SetCommand=cmd(settext,string.upper( SortOrderToLocalizedString(GAMESTATE:GetSortOrder()) ));
			SortOrderChangedMessageCommand=cmd(playcommand,"Set");
		};
		LoadFont( "_sf square head bold 28" ) .. {
			InitCommand=cmd(zoom,0.5;maxwidth,300;playcommand,"Set";x,10;shadowlength,0;diffuse,color("#fbfb57"););
			SetCommand=cmd(settext,string.upper( SortOrderToLocalizedString(GAMESTATE:GetSortOrder()) ));
			SortOrderChangedMessageCommand=cmd(playcommand,"Set");
		};
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
		InitCommand=cmd(player,PLAYER_1;playcommand,"Set";x,SCREEN_CENTER_X-180;y,SCREEN_CENTER_Y+194);
		
		SetCommand=function(self) self:SetFromGameState() end;
		CurrentStepsP1ChangedMessageCommand=cmd(playcommand,"Set");
		CurrentTrailP1ChangedMessageCommand=cmd(playcommand,"Set");
		SortOrderChangedMessageCommand=cmd(playcommand,"Set");
	};

	Def.PaneDisplay {
		MetricsGroup="PaneDisplay";
		PlayerNumber=PLAYER_2;
		InitCommand=cmd(player,PLAYER_2;playcommand,"Set";x,SCREEN_CENTER_X+180;y,SCREEN_CENTER_Y+194);
		
		SetCommand=function(self) self:SetFromGameState() end;
		CurrentStepsP2ChangedMessageCommand=cmd(playcommand,"Set");
		CurrentTrailP2ChangedMessageCommand=cmd(playcommand,"Set");
		SortOrderChangedMessageCommand=cmd(playcommand,"Set");
	};

	Def.BPMDisplay {
		File=THEME:GetPathF("BPMDisplay", "bpm");
		Name="BPMDisplay";
		InitCommand=cmd(horizalign,right;x,SCREEN_CENTER_X+294;y,SCREEN_CENTER_Y+1;shadowlengthx,0;shadowlengthy,2;shadowcolor,color("#000000");zoom,0.5;);
		OnCommand=cmd(stoptweening;addx,-SCREEN_WIDTH*0.6;bounceend,0.5;addx,SCREEN_WIDTH*0.6);
		OffCommand=cmd(bouncebegin,0.5;addx,-SCREEN_WIDTH*0.6);
		SetCommand=function(self) self:SetFromGameState() end;
		CurrentSongChangedMessageCommand=cmd(playcommand,"Set");
		CurrentCourseChangedMessageCommand=cmd(playcommand,"Set");
	};
	LoadActor( "_bpm label" ) .. {
		InitCommand=cmd(horizalign,left;x,SCREEN_CENTER_X+280;y,SCREEN_CENTER_Y);
		OnCommand=cmd(addx,-SCREEN_WIDTH*0.6;bounceend,0.5;addx,SCREEN_WIDTH*0.6);
		OffCommand=cmd(bouncebegin,0.5;addx,-SCREEN_WIDTH*0.6);
	};
	LoadActor( "bpm meter" ) .. {
		InitCommand=cmd(x,SCREEN_CENTER_X+230;y,SCREEN_CENTER_Y-12);
		OnCommand=cmd(addx,-SCREEN_WIDTH*0.6;bounceend,0.5;addx,SCREEN_WIDTH*0.6);
		OffCommand=cmd(bouncebegin,0.5;addx,-SCREEN_WIDTH*0.6);
	};
	LoadActor( "stop icon" ) .. {
		InitCommand=cmd(x,SCREEN_CENTER_X+296;y,SCREEN_CENTER_Y+6);
		OnCommand=cmd(addx,-SCREEN_WIDTH*0.6;bounceend,0.5;addx,SCREEN_WIDTH*0.6);
		OffCommand=cmd(bouncebegin,0.5;addx,-SCREEN_WIDTH*0.6);
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
	
	LoadFont("_regra 32 bold") .. {
		InitCommand=cmd(horizalign,left;x,SCREEN_CENTER_X-14;y,SCREEN_CENTER_Y-14;zoom,0.5;settext,"/Senser";shadowlengthx,0;shadowlengthy,2;shadowcolor,color("#000000"););
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
	};
	LoadFont("_regra 32 bold") .. {
		InitCommand=cmd(horizalign,right;x,SCREEN_CENTER_X+228;y,SCREEN_CENTER_Y+4;zoom,0.5;settext,"_Hardcore Techno";shadowlengthx,0;shadowlengthy,2;shadowcolor,color("#000000"););
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

	Def.ActorFrame {
		InitCommand=cmd(x,SCREEN_CENTER_X+26;y,SCREEN_CENTER_Y+5;);
		LoadActor("star full") .. { InitCommand=cmd(x,16*-2); };
		LoadActor("star full") .. { InitCommand=cmd(x,16*-1); };
		LoadActor("star full") .. { InitCommand=cmd(x,16*0); };
		LoadActor("star empty") .. { InitCommand=cmd(x,16*1); };
		LoadActor("star empty") .. { InitCommand=cmd(x,16*2); };
		CurrentSongChangedMessageCommand=cmd(playcommand,"Set");
		CurrentCourseChangedMessageCommand=cmd(playcommand,"Set");
	};
	
	Def.CourseContentsList {
		MaxSongs = 5;

		InitCommand=cmd(x,SCREEN_CENTER_X-160;y,SCREEN_CENTER_Y+96);
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
				ArtistPrependString="/";
				SetCommand=TextBannerSet;
				InitCommand=cmd(LoadFromString,"", "", "", "", "", "");
				Title = LoadFont("TextBanner","text") .. {
					Name="Title";
					OnCommand=cmd(shadowlength,0);
				};
				Subtitle = LoadFont("TextBanner","text") .. {
					Name="Subtitle";
					OnCommand=cmd(shadowlength,0);
				};
				Artist = LoadFont("TextBanner","text") .. {
					Name="Artist";
					OnCommand=cmd(shadowlength,0);
				};
				SetSongCommand=function(self, params)
					if params.Song then
						self:LoadFromSong( params.Song );
						self:diffuse( SONGMAN:GetSongColor(params.Song) );
					else
						self:LoadFromString( "??????????", "??????????", "", "", "", "" );
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
				OnCommand=cmd(x,SCREEN_CENTER_X-192;y,SCREEN_CENTER_Y-230;horizalign,right;zoom,0.5;shadowlength,0);
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

	Autogen(PLAYER_1) .. {
		BeginCommand=cmd(x,SCREEN_CENTER_X-290;y,SCREEN_CENTER_Y-15);
	};
	Autogen(PLAYER_2) .. {
		BeginCommand=cmd(x,SCREEN_CENTER_X-68;y,SCREEN_CENTER_Y-15);
	};
	
	--DifficultyIcons(PLAYER_1) .. {
	--	BeginCommand=cmd(x,SCREEN_CENTER_X-290;y,SCREEN_CENTER_Y-15);
	--	OnCommand=cmd(zoomy,0;linear,0.5;zoomy,1);
	--	OffCommand=cmd(linear,0.5;zoomy,0);
	--};
	--DifficultyIcons(PLAYER_2) .. {
	--	BeginCommand=cmd(x,SCREEN_CENTER_X-68;y,SCREEN_CENTER_Y-15);
	--	OnCommand=cmd(zoomy,0;linear,0.5;zoomy,1);
	--	OffCommand=cmd(linear,0.5;zoomy,0);
	--};

	--DifficultyDisplay(PLAYER_1) .. {
	--	BeginCommand=cmd(player,PLAYER_1;x,SCREEN_CENTER_X-254;y,SCREEN_CENTER_Y-20);
	--	OnCommand=cmd(zoomy,0;linear,0.5;zoomy,1);
	--	OffCommand=cmd(linear,0.5;zoomy,0);
	--};
	--DifficultyDisplay(PLAYER_2) .. {
	--	BeginCommand=cmd(player,PLAYER_2;x,SCREEN_CENTER_X-121;y,SCREEN_CENTER_Y-20);
	--	OnCommand=cmd(zoomy,0;linear,0.5;zoomy,1);
	--	OffCommand=cmd(linear,0.5;zoomy,0);
	--};

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

	LoadFont("_numbers2") .. {
		InitCommand=cmd(x,SCREEN_CENTER_X+262;y,SCREEN_CENTER_Y);
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
	--LoadActor( "difficulties" ) .. {
	--	InitCommand=cmd(x,SCREEN_CENTER_X-35;y,SCREEN_CENTER_Y-134);
	--	OnCommand=cmd(finishtweening;addx,-SCREEN_WIDTH*0.6;bounceend,0.5;addx,SCREEN_WIDTH*0.6);
	--	OffCommand=cmd(finishtweening;bouncebegin,0.5;addx,-SCREEN_WIDTH*0.6);
	--};
};

if not GAMESTATE:IsCourseMode() then
	t[#t+1] = Def.DifficultyList {
		Name="DifficultyList";
		OnCommand=cmd(x,SCREEN_CENTER_X+170;y,SCREEN_CENTER_Y+40);
		CursorP1 = Def.ActorFrame {
			InitCommand=cmd(x,-150;bounce;effectmagnitude,-5,0,0;effectperiod,1.0;effectoffset,0.0;effectclock,"bgm");
			BeginCommand=cmd(visible,true);
			StepsSelectedMessageCommand=function( self, param ) 
				if param.Player ~= "PlayerNumber_P1" then return end;
				self:visible(false);
			end;
			children={
				LoadActor( "DifficultyList cursor p1" ) .. {
					BeginCommand=cmd(player,"PlayerNumber_P1";);
					PlayerJoinedMessageCommand=function(self,param )
						if param.Player ~= "PlayerNumber_P1" then return end;
						self:visible( true );
					end;
				};
				LoadFont( "_terminator two 32" ) .. {
					InitCommand=cmd(x,-4;y,-3;settext,"P1";zoom,0.5;diffuse,color("#baa200");shadowlength,0;strokecolor,color("#00000000"););
					BeginCommand=cmd(player,"PlayerNumber_P1";);
					PlayerJoinedMessageCommand=function(self,param )
						if param.Player ~= "PlayerNumber_P1" then return end;
						self:visible( true );
					end;
				};
			}
		};
		CursorP2 = Def.ActorFrame {
			InitCommand=cmd(x,130;bounce;effectmagnitude,5,0,0;effectperiod,1.0;effectoffset,0.0;effectclock,"bgm");
			BeginCommand=cmd(visible,true);
			StepsSelectedMessageCommand=function( self, param ) 
				if param.Player ~= "PlayerNumber_P2" then return end;
				self:visible(false);
			end;
			children={
				LoadActor( "DifficultyList cursor p2" ) .. {
					BeginCommand=cmd(player,"PlayerNumber_P2";);
					PlayerJoinedMessageCommand=function(self,param )
						if param.Player ~= "PlayerNumber_P2" then return end;
						self:visible( true );
					end;
				};
				LoadFont( "_terminator two 32" ) .. {
					InitCommand=cmd(x,4;y,-3;settext,"P2";zoom,0.5;diffuse,color("#83b767");shadowlength,0;strokecolor,color("#00000000"););
					BeginCommand=cmd(player,"PlayerNumber_P2";);
					PlayerJoinedMessageCommand=function(self,param )
						if param.Player ~= "PlayerNumber_P2" then return end;
						self:visible( true );
					end;
				};
			}
		};
		CursorP1Frame = LoadActor( "DifficultyList frame p1" ) .. {
			InitCommand=cmd(bounce;effectmagnitude,-10,0,0;effectperiod,1.0;effectoffset,0.0;effectclock,"bgm");
		};
		CursorP2Frame = LoadActor( "DifficultyList frame p2" ) .. {
			InitCommand=cmd(bounce;effectmagnitude,10,0,0;effectperiod,1.0;effectoffset,0.0;effectclock,"bgm");
		};
	};
end

return t;
