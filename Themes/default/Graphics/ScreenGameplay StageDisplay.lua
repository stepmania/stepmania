local curScreen = Var "LoadingScreen";
local curStage = GAMESTATE:GetCurrentStage();
local curStageIndex = GAMESTATE:GetCurrentStageIndex();
local playMode = GAMESTATE:GetPlayMode();

local tRemap = {
	Stage_1st		= 1,
	Stage_2nd		= 2,
	Stage_3rd		= 3,
	Stage_4th		= 4,
	Stage_5th		= 5,
	Stage_6th		= 6,
};
if tRemap[curStage] == PREFSMAN:GetPreference("SongsPerPlay") then
	curStage = "Stage_Final";
end

local t = Def.ActorFrame {
	LoadActor(THEME:GetPathB("_frame","3x3"),"rounded black",64,16);
	LoadFont("Common Normal") .. {
		InitCommand=cmd(y,-1;shadowlength,1;playcommand,"Set");
		CurrentSongChangedMessageCommand=cmd(playcommand,"Set");
		CurrentCourseChangedMessageCommand=cmd(playcommand,"Set");
		CurrentStepsP1ChangedMessageCommand=cmd(playcommand,"Set");
		CurrentStepsP2ChangedMessageCommand=cmd(playcommand,"Set");
		CurrentTraiP1ChangedMessageCommand=cmd(playcommand,"Set");
		CurrentTraiP2ChangedMessageCommand=cmd(playcommand,"Set");
		SetCommand=function(self)
			if playMode ~= 'PlayMode_Regular' and playMode ~= 'PlayMode_Battle' and playMode ~= 'PlayMode_Rave' then
				self:settextf("%i / %i", tonumber(curStageIndex) + 1, GAMESTATE:GetCurrentCourse():GetEstimatedNumStages());
			else
				if GAMESTATE:IsEventMode() then
					self:settextf("Stage %s", curStageIndex+1);
				else
					if THEME:GetMetric(curScreen,"StageDisplayUseShortString") then
						self:settextf("%s", ToEnumShortString(curStage));
					else
						self:settextf("%s Stage", ToEnumShortString(curStage));
					end
				end
			end;
			self:zoom(0.675);
			self:diffuse(StageToColor(curStage));
			self:diffusetopedge(ColorLightTone(StageToColor(curStage)));
		end;
	};
};
return t