local t = Def.ActorFrame {};
	-- Bar
	t[#t+1] = LoadActor(THEME:GetPathG("LifeMeter", "p1 bar")) .. {
	};

	-- Difficulty

	t[#t+1] = Def.ActorFrame {
		InitCommand=cmd(visible,GAMESTATE:IsHumanPlayer(PLAYER_1);x,-207;y,0;);
		LoadActor("_diffdia") .. {
		OnCommand=cmd(playcommand,"Set";);
		CurrentStepsP1ChangedMessageCommand=cmd(playcommand,"Set";);
		SetCommand=function(self)
			stepsP1 = GAMESTATE:GetCurrentSteps(PLAYER_1)
			local song = GAMESTATE:GetCurrentSong();
			if song then
				if stepsP1 ~= nil then
					local st = stepsP1:GetStepsType();
					local diff = stepsP1:GetDifficulty();
					local cd = GetCustomDifficulty(st, diff);
					self:diffuse(CustomDifficultyToColor(cd));
				end
			end
		end;
		};
		LoadFont("StepsDisplay description") .. {
			  InitCommand=cmd(zoom,0.75;horizalign,center;rotationz,90);
			  OnCommand=cmd(playcommand,"Set";);
			  CurrentStepsP1ChangedMessageCommand=cmd(playcommand,"Set";);
			  ChangedLanguageDisplayMessageCommand=cmd(playcommand,"Set");
			  SetCommand=function(self)
				stepsP1 = GAMESTATE:GetCurrentSteps(PLAYER_1)
				local song = GAMESTATE:GetCurrentSong();
				if song then
					if stepsP1 ~= nil then
						local st = stepsP1:GetStepsType();
						local diff = stepsP1:GetDifficulty();
						local cd = GetCustomDifficulty(st, diff);
						self:settext(stepsP1:GetMeter())
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
