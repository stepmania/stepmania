local t = LoadActor("ScreenWithMenuElements stage/stage" ) .. {
	BeginCommand=cmd(playcommand,"Set", {
		StageToShow = SCREENMAN:GetTopScreen():GetStageStats():GetStage();
		StageNumber = SCREENMAN:GetTopScreen():GetStageStats():GetStageIndex();
	} );
};
return t;
