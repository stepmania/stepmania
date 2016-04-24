return Def.ActorFrame {
	Def.Quad{
		Name="TopMask";
		--InitCommand=cmd(x,8;y,-68;zoomto,296,90;valign,1;clearzbuffer,true;zwrite,true;blend,Blend.NoEffect;);
		InitCommand=cmd(x,8;y,-69;zoomto,296,90;valign,1;clearzbuffer,true;zwrite,true;blend,Blend.NoEffect;);
	};
	Def.Quad{
		Name="BottomMask";
		InitCommand=cmd(x,8;y,68;zoomto,296,90;valign,0;clearzbuffer,false;zwrite,true;blend,Blend.NoEffect;);
	};
	Def.CourseContentsList {
		MaxSongs = 30; -- any course with over 30 songs?? idk
		NumItemsToDraw = 3.15;
		SetCommand=function(self)
			self:finishtweening();
			self:SetFromGameState();
			self:setsize(296,180);
			self:SetCurrentAndDestinationItem(1);
			-- todo: only true if course has > 3 songs.
			-- xxx: allows weird setups to happen (Song 3, Song 1, Song 2)
			local course = GAMESTATE:GetCurrentCourse();
			if course:GetEstimatedNumStages() < 4 then
				self:SetLoop(false);
			else
				self:SetLoop(true);
				self:scrollthroughallitems();
			end;
		end;
		CurrentCourseChangedMessageCommand=function(self)
			local course = GAMESTATE:GetCurrentCourse();
			self:visible( course and true or false );
		end;
		CurrentTrailP1ChangedMessageCommand=cmd(playcommand,"Set");
		CurrentTrailP2ChangedMessageCommand=cmd(playcommand,"Set");

		Display = Def.ActorFrame{ 
			InitCommand=cmd(setsize,296,44);

			Def.TextBanner {
				InitCommand=cmd(x,-96;y,-2;Load,"CourseDisplayTextBanner";SetFromString,"", "", "", "", "", "");
				SetSongCommand=function(self, params)
					if params.Song then
						self:SetFromSong( params.Song );
						self:diffuse( SONGMAN:GetSongColor(params.Song) );
					else
						self:SetFromString( "??????????", "??????????", "", "", "", "" );
						self:diffuse( color("#FFFFFF") );
					end
				end;
				OffCommand=cmd(linear,0.2;diffusealpha,0);
			};

			Font("mentone","24px")..{
				InitCommand=cmd(x,-132;y,-15;shadowlength,1;halign,0;zoom,0.625;strokecolor,color("0,0,0,0"));
				SetSongCommand=function(self, params) 
					self:settext(string.format("#%i", params.Number)); 
				end;
				OffCommand=cmd(linear,0.2;diffusealpha,0);
			};
			Font("mentone","24px")..{
				Text="0";
				InitCommand=cmd(x,-114;y,3;skewx,-0.15;zoom,1;shadowlength,1;strokecolor,color("0,0,0,0"));
				SetSongCommand=function(self, params)
					if params.PlayerNumber ~= GAMESTATE:GetMasterPlayerNumber() then return end
					self:settext( params.Meter );
					self:diffuse( CustomDifficultyToColor(params.Difficulty) );
				end;
				OffCommand=cmd(linear,0.2;diffusealpha,0);
			}; 

			Def.Quad{
				Name="SongSplitter";
				InitCommand=cmd(x,8;y,20;diffusealpha,0.5;zoomto,288,2;ztest,true);
				OffCommand=cmd(linear,0.2;zoomx,0);
			};
		};
	};
};