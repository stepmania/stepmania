local stages = Def.ActorFrame {
	BeginCommand=cmd(playcommand,"Set";);
	CurrentSongChangedMessageCommand=cmd(finishtweening;playcommand,"Set";);
};
local curScreen = Var "LoadingScreen";
local curStage = GAMESTATE:GetCurrentStage();
local curStageIndex = GAMESTATE:GetCurrentStageIndex();

local t = Def.ActorFrame {};
t[#t+1] = Def.ActorFrame {
 	LoadActor(THEME:GetPathB("_frame","3x3"),"rounded black",160,20);
	LoadFont("Common Normal") .. {
		Text=StageToLocalizedString( curStage ) .. " Stage";
--~ 		Text=StageToLocalizedString( curStage ) .. " Stage (" .. curStageIndex .. ")";
		InitCommand=cmd(shadowlength,1;
			diffuse,StageToColor(curStage);
			diffusetopedge,ColorLightTone(StageToColor(curStage));
			-- shadowcolor,ColorDarkTone(StageToColor(curStage));
		);
	};
};
return t