return Def.CourseContentsList {
	MaxSongs = 4;
    NumItemsToDraw = 8; -- xxx: Doesn't scroll anymore.
	ShowCommand=function(self)
		self:bouncebegin(0.3);
		self:zoomy(1);
	end;
	HideCommand=function(self)
		self:linear(0.3);
		self:zoomy(0);
	end;
	SetCommand=function(self)
		self:SetFromGameState();
		self:PositionItems();
		self:SetTransformFromHeight(44);
		self:SetCurrentAndDestinationItem(0);
		self:SetLoop(false);
		self:SetMask(270,0);
	end;
	CurrentTrailP1ChangedMessageCommand=function(self)
		self:playcommand("Set");
	end;
	CurrentTrailP2ChangedMessageCommand=function(self)
		self:playcommand("Set");
	end;

	Display = Def.ActorFrame { 
		InitCommand=function(self)
			self:setsize(270,44);
		end;

		LoadActor(THEME:GetPathG("CourseEntryDisplay","bar")) .. {
			SetSongCommand=function(self, params)
				if params.Difficulty then
					self:diffuse( CustomDifficultyToColor(params.Difficulty) );
				else
					self:diffuse( color("#FFFFFF") );
				end

				self:finishtweening();
				self:diffusealpha(0);
				self:sleep(0.125 * params.Number);
				self:linear(0.125);
				self:diffusealpha(1);
				self:linear(0.05);
				self:glow(color("1,1,1,0.5"));
				self:decelerate(0.1);
				self:glow(color("1,1,1,0"));
			end;
		};

		Def.TextBanner {
			InitCommand=function(self)
				self:x(-128);
				self:y(1);
				self:Load("TextBanner");
				self:SetFromString("", "", "", "", "", "");
			end;
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
				else
					self:SetFromString( "??????????", "??????????", "", "", "", "" );
					self:diffuse( color("#FFFFFF") );
				end
				
				self:finishtweening();
				self:zoomy(0);
				self:sleep(0.125 * params.Number);
				self:linear(0.125);
				self:zoomy(1.1);
				self:linear(0.05);
				self:zoomx(1.1);
				self:decelerate(0.1);
				self:zoom(1);
			end;
		};

 		LoadFont("CourseEntryDisplay","difficulty") .. {
			Text="0";
			InitCommand=function(self)
				self:x(114);
				self:y(0);
				self:zoom(0.75);
				self:shadowlength(1);
			end;
			SetSongCommand=function(self, params)
				if params.PlayerNumber ~= GAMESTATE:GetMasterPlayerNumber() then return end
				self:settext( params.Meter );
				self:diffuse( CustomDifficultyToColor(params.Difficulty) );
				self:finishtweening();
				self:zoomy(0);
				self:sleep(0.125 * params.Number);
				self:linear(0.125);
				self:zoomy(1.1);
				self:linear(0.05);
				self:zoomx(1.1);
				self:decelerate(0.1);
				self:zoom(1);
			end;
		}; 
	};
};