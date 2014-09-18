local curScreen = Var "LoadingScreen";
local curStageIndex = GAMESTATE:GetCurrentStageIndex();
local t = Def.ActorFrame {};

t[#t+1] = Def.ActorFrame {
	LoadFont("Common Normal") .. {
		InitCommand=cmd(y,-1;shadowlength,1;);
		BeginCommand=function(self)
			local top = SCREENMAN:GetTopScreen()
			if top then
				if not string.find(top:GetName(),"ScreenEvaluation") then
					curStageIndex = curStageIndex + 1
				end
			end
			self:playcommand("Set")
		end;
		CurrentSongChangedMessageCommand= cmd(playcommand,"Set"),
		SetCommand=function(self)
			local curStage = GAMESTATE:GetCurrentStage();
			if GAMESTATE:GetCurrentCourse() then
				self:settext( curStageIndex+1 .. " / " .. GAMESTATE:GetCurrentCourse():GetEstimatedNumStages() );
			elseif GAMESTATE:IsEventMode() then
				self:settextf("Stage %s", curStageIndex);
			else
				local thed_stage= thified_curstage_index(curScreen:find("Evaluation"))
				if THEME:GetMetric(curScreen,"StageDisplayUseShortString") then
					self:settext(thed_stage)
					self:zoom(0.75);
				else
					self:settextf("%s Stage", thed_stage);
					self:zoom(1);
				end;
			end;
			-- StepMania is being stupid so we have to do this here;
			self:diffuse(StageToColor(curStage));
			self:diffusetopedge(ColorLightTone(StageToColor(curStage)));
		end;
	};
};
return t
