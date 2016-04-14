local t = Def.ActorFrame{
	-- lol the things I have to hack in to fix StepMania's oversights (and yes,
	-- this fix applies to sm-ssc v1.0 beta 2 [also beta 3, likely] as well.)
	Def.Actor{
		Name="FixerUpper";
		CurrentSongChangedMessageCommand=function(self)
			local song = GAMESTATE:GetCurrentSong();
			for pn in ivalues(PlayerNumber) do
				local score = SCREENMAN:GetTopScreen():GetChild("Score"..ToEnumShortString(pn));
				if score and not song then
					score:settext("        0");
				end;
			end;
		end;
	};
};

t[#t+1] = StandardDecorationFromFile("ArtistAndGenre","ArtistAndGenre");
t[#t+1] = StandardDecorationFromFile("BPMDisplay","BPMDisplay");
t[#t+1] = StandardDecorationFromFileOptional("SortDisplay","SortDisplay");
t[#t+1] = StandardDecorationFromFileOptional("SelectionLength","SelectionLength");
t[#t+1] = StandardDecorationFromFileOptional("SongOptions","SongOptions");
t[#t+1] = StandardDecorationFromFileOptional("StageDisplay","StageDisplay");
t[#t+1] = StandardDecorationFromFileOptional("CourseContentsList","CourseContentsList");

-- todo: optimize heavily
if not GAMESTATE:IsCourseMode() then
	t[#t+1] = Def.StepsDisplayList {
		Name="StepsDisplayList";
		InitCommand=cmd(xy,(SCREEN_CENTER_X*0.75/2)+28,SCREEN_CENTER_Y*1.275);
		OffCommand=cmd(bouncebegin,0.375;addx,-SCREEN_CENTER_X*1.25);
		CurrentSongChangedMessageCommand=function(self)
			self:visible(GAMESTATE:GetCurrentSong() ~= nil);
		end;
		CursorP1 = Def.ActorFrame {
			BeginCommand=cmd(visible,true);
			StepsSelectedMessageCommand=function( self, param ) 
				if param.Player ~= "PlayerNumber_P1" then return end;
				self:visible(false);
			end;
			children={
				LoadActor( "StepsDisplayList highlight" ) .. {
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
					InitCommand=cmd(x,-130;);
					children={
						Font("mentone","24px") .. {
							InitCommand=cmd(y,-3;settext,"P1";diffuse,PlayerColor("PlayerNumber_P1");shadowlength,1;zoom,0.5;shadowcolor,color("#00000044");NoStroke);
							BeginCommand=cmd(player,"PlayerNumber_P1";);
							PlayerJoinedMessageCommand=function(self,param )
								if param.Player ~= "PlayerNumber_P1" then return end;
								self:visible( true );
							end;
						};
					}
				};
			};
		};
		CursorP2 = Def.ActorFrame {
			BeginCommand=cmd(visible,true);
			StepsSelectedMessageCommand=function( self, param ) 
				if param.Player ~= "PlayerNumber_P2" then return end;
				self:visible(false);
			end;
			children={
				LoadActor( "StepsDisplayList highlight" ) .. {
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
					InitCommand=cmd(x,-127;);
					children={
						Font("mentone","24px") .. {
							InitCommand=cmd(y,3;settext,"P2";diffuse,PlayerColor("PlayerNumber_P2");shadowlength,1;zoom,0.5;shadowcolor,color("#00000044");NoStroke);
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
        CursorP3 = Def.ActorFrame {
			BeginCommand=cmd(visible,true);
			StepsSelectedMessageCommand=function( self, param ) 
				if param.Player ~= "PlayerNumber_P3" then return end;
				self:visible(false);
			end;
			children={
				LoadActor( "StepsDisplayList highlight" ) .. {
					InitCommand=cmd(addx,-10;zoomx,-1;diffusealpha,0.3);
					BeginCommand=cmd(player,"PlayerNumber_P3");
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
						if param.Player ~= "PlayerNumber_P3" then return end;
						self:visible( true );
					end;
				};
				Def.ActorFrame {
					InitCommand=cmd(x,-127;);
					children={
						Font("mentone","24px") .. {
							InitCommand=cmd(y,3;settext,"P3";diffuse,PlayerColor("PlayerNumber_P3");shadowlength,1;zoom,0.5;shadowcolor,color("#00000044");NoStroke);
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
        CursorP4 = Def.ActorFrame {
			BeginCommand=cmd(visible,true);
			StepsSelectedMessageCommand=function( self, param ) 
				if param.Player ~= "PlayerNumber_P4" then return end;
				self:visible(false);
			end;
			children={
				LoadActor( "StepsDisplayList highlight" ) .. {
					InitCommand=cmd(addx,-10;zoomx,-1;diffusealpha,0.3);
					BeginCommand=cmd(player,"PlayerNumber_P4");
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
						if param.Player ~= "PlayerNumber_P4" then return end;
						self:visible( true );
					end;
				};
				Def.ActorFrame {
					InitCommand=cmd(x,-127;);
					children={
						Font("mentone","24px") .. {
							InitCommand=cmd(y,3;settext,"P4";diffuse,PlayerColor("PlayerNumber_P4");shadowlength,1;zoom,0.5;shadowcolor,color("#00000044");NoStroke);
							BeginCommand=cmd(player,"PlayerNumber_P4";);
							PlayerJoinedMessageCommand=function(self,param )
								if param.Player ~= "PlayerNumber_P4" then return end;
								self:visible( true );
							end;
						};
					}
				};
			}
		};
		CursorP1Frame = Def.Actor{ };
		CursorP2Frame = Def.Actor{ };
        CursorP3Frame = Def.Actor{ };
        CursorP4Frame = Def.Actor{ };
	};
end

return t;
