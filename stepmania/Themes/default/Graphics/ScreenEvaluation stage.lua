local t = LoadActor("ScreenWithMenuElements stage/stage" ) .. {
	BeginCommand=cmd(playcommand,"Set", { StageToShow = SCREENMAN:GetTopScreen():GetStageStats():GetStage() } );
};
return t;
