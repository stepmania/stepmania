local t = Def.ActorFrame {};
	-- Bar
	t[#t+1] = LoadActor(THEME:GetPathG("LifeMeter", "p1 bar")) .. {
	};

	-- Difficulty

	t[#t+1] = Def.ActorFrame {
		InitCommand=cmd(visible,GAMESTATE:IsHumanPlayer(PLAYER_2);x,-207;y,0;);
		LoadActor("_diffdia") .. {
		OnCommand=cmd(playcommand,"Set";);
		CurrentstepsP2ChangedMessageCommand=cmd(playcommand,"Set";);
		SetCommand=function(self)
			stepsP2 = GAMESTATE:GetCurrentSteps(PLAYER_2)
			local song = GAMESTATE:GetCurrentSong();
			if song then
				if stepsP2 ~= nil then
					local st = stepsP2:GetStepsType();
					local diff = stepsP2:GetDifficulty();
					local cd = GetCustomDifficulty(st, diff);
					self:diffuse(CustomDifficultyToColor(cd));
				end
			end
		end;
		};
		LoadFont("StepsDisplay description") .. {
			  InitCommand=cmd(zoom,0.75;horizalign,center;rotationz,90);
			  OnCommand=cmd(playcommand,"Set";);
			  CurrentstepsP2ChangedMessageCommand=cmd(playcommand,"Set";);
			  ChangedLanguageDisplayMessageCommand=cmd(playcommand,"Set");
			  SetCommand=function(self)
				stepsP2 = GAMESTATE:GetCurrentSteps(PLAYER_2)
				local song = GAMESTATE:GetCurrentSong();
				if song then
					if stepsP2 ~= nil then
						local st = stepsP2:GetStepsType();
						local diff = stepsP2:GetDifficulty();
						local cd = GetCustomDifficulty(st, diff);
						self:settext(stepsP2:GetMeter())
						self:diffuse(ColorDarkTone(CustomDifficultyToColor(cd)));
					else
						self:settext("")
					end
				else
					self:settext("")
				end
			  end
		};
	};

return t;
