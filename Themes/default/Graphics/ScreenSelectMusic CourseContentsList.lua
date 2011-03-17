return Def.CourseContentsList {
	MaxSongs = 4;
    NumItemsToDraw = 8; -- xxx: Doesn't scroll anymore.
--[[ 	InitCommand=cmd(x,SCREEN_CENTER_X+160;y,SCREEN_CENTER_Y+91);
 	OnCommand=cmd(zoomy,0;bounceend,0.3;zoom,1);
	OffCommand=cmd(zoomy,1;bouncebegin,0.3;zoomy,0); --]]
	ShowCommand=cmd(bouncebegin,0.3;zoomy,1);
	HideCommand=cmd(linear,0.3;zoomy,0);
	SetCommand=function(self)
		self:SetFromGameState();
		self:PositionItems();
		self:SetTransformFromHeight(44);
		self:SetCurrentAndDestinationItem(0);
-- 		self:SetDestinationItem( self:GetNumItems()-2 );
		self:SetLoop(false);
		self:SetMask(270,0);
	end;
	CurrentTrailP1ChangedMessageCommand=cmd(playcommand,"Set");
	CurrentTrailP2ChangedMessageCommand=cmd(playcommand,"Set");

	Display = Def.ActorFrame { 
		InitCommand=cmd(setsize,270,44);

		LoadActor(THEME:GetPathG("CourseEntryDisplay","bar")) .. {
-- 			InitCommand=cmd(diffusetopedge,Color("Invisible"));
			SetSongCommand=function(self, params)
				if params.Difficulty then
-- 					self:diffuse( SONGMAN:GetSongColor(params.Song) );
					self:diffuse( CustomDifficultyToColor(params.Difficulty) );
				else
					self:diffuse( color("#FFFFFF") );
-- 					self:diffuse( CustomDifficultyToColor(params.Difficulty) );
				end

				(cmd(finishtweening;diffusealpha,0;sleep,0.125*params.Number;linear,0.125;diffusealpha,1;linear,0.05;glow,color("1,1,1,0.5");decelerate,0.1;glow,color("1,1,1,0")))(self);
			end;
		};

		Def.TextBanner {
			InitCommand=cmd(x,-128;y,1;Load,"TextBanner";SetFromString,"", "", "", "", "", "");
			SetSongCommand=function(self, params)
				if params.Song then
					if GAMESTATE:GetCurrentCourse():GetDisplayFullTitle() == "Abomination" then
						-- abomination hack
						if PREFSMAN:GetPreference("EasterEggs") then
							if params.Number % 2 ~= 0 then
								-- turkey march
								local artist = params.Song:GetDisplayArtist();
								self:SetFromString( "Turkey", "", "", "", artist, "" );
							else
								self:SetFromSong( params.Song );
							end;
						else
							self:SetFromSong( params.Song );
						end;
					else
						self:SetFromSong( params.Song );
					end;
					self:diffuse( CustomDifficultyToColor(params.Difficulty) );
-- 					self:glow("1,1,1,0.5");
				else
					self:SetFromString( "??????????", "??????????", "", "", "", "" );
					self:diffuse( color("#FFFFFF") );
-- 					self:glow("1,1,1,0");
				end
				
				(cmd(finishtweening;zoomy,0;sleep,0.125*params.Number;linear,0.125;zoomy,1.1;linear,0.05;zoomx,1.1;decelerate,0.1;zoom,1))(self);
			end;
		};

--[[ 		LoadFont("CourseEntryDisplay","number") .. {
			InitCommand=cmd(x,114+8;y,-12;shadowlength,1);
			SetSongCommand=function(self, params) 
				self:settext(string.format("#%i", params.Number)); 

				(cmd(finishtweening;zoom,0.5;zoomy,0.5*1.5;diffusealpha,0;sleep,0.125*params.Number;linear,0.125;diffusealpha,1;linear,0.05;zoomy,0.5*1;zoomx,0.5*1.1;glow,color("1,1,1,0.5");decelerate,0.1;zoom,0.5;glow,color("1,1,1,0")))(self);
			end;
		}; --]]
 		LoadFont("CourseEntryDisplay","difficulty") .. {
			Text="0";
			InitCommand=cmd(x,114;y,0;zoom,0.75;shadowlength,1);
			SetSongCommand=function(self, params)
				if params.PlayerNumber ~= GAMESTATE:GetMasterPlayerNumber() then return end
				self:settext( params.Meter );
				self:diffuse( CustomDifficultyToColor(params.Difficulty) );
				(cmd(finishtweening;zoomy,0;sleep,0.125*params.Number;linear,0.125;zoomy,1.1;linear,0.05;zoomx,1.1;decelerate,0.1;zoom,1))(self);
			end;
		}; 
--[[ 		LoadFont("Common","normal") .. {
			OnCommand=cmd(x,0;y,-8;zoom,0.7;shadowlength,0);
			DifficultyChangedCommand=function(self, params)
				if params.PlayerNumber ~= GAMESTATE:GetMasterPlayerNumber() then return end
				self:settext( params.Meter );
				self:diffuse( CourseDifficultyColors[params.Difficulty] );
			end;
		}; --]]

--[[ 		LoadFont("Common","normal") .. {
			OnCommand=cmd(x,SCREEN_CENTER_X-192;y,SCREEN_CENTER_Y-230;horizalign,right;shadowlength,0);
			SetSongCommand=function(self, params) self:settext(params.Modifiers); end;
		}; --]]

	};
};