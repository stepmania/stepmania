local curScreen = Var "LoadingScreen";
local curStage = GAMESTATE:GetCurrentStage();
local curStageIndex = GAMESTATE:GetCurrentStageIndex();

local t = Def.ActorFrame {};

t[#t+1] = Def.ActorFrame {
	LoadActor(THEME:GetPathB("_frame","3x3"),"rounded black",64,16);
	LoadFont("Common Normal") .. {
		InitCommand=cmd(y,-1;shadowlength,1;playcommand,"Set");
        SetCommand=function(self)
          if GAMESTATE:GetCurrentCourse() then
            self:settext( GAMESTATE:GetCurrentStageIndex()+1 .. " / " .. GAMESTATE:GetCurrentCourse():GetEstimatedNumStages() );
          elseif GAMESTATE:IsEventMode() then
            self:settextf("Stage %s", curStageIndex+1);
          else
            if THEME:GetMetric(curScreen,"StageDisplayUseShortString") then
              self:settextf("%s", ToEnumShortString(curStage));
            else
              self:settextf("%s Stage", ToEnumShortString(curStage));
            end;
          end;
          --
          self:zoom(0.675);
          self:diffuse(StageToColor(curStage));
          self:diffusetopedge(ColorLightTone(StageToColor(curStage)));
        end;
	};
};
return t