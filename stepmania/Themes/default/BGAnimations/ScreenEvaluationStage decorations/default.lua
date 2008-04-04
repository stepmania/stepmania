local t = LoadFallbackB( "decorations" );

local p1x = THEME:GetMetric( 'ScreenEvaluationStage', 'BonusFrameP1X' )
local p1y = THEME:GetMetric( 'ScreenEvaluationStage', 'BonusFrameP1Y' )
local p2x = THEME:GetMetric( 'ScreenEvaluationStage', 'BonusFrameP2X' )
local p2y = THEME:GetMetric( 'ScreenEvaluationStage', 'BonusFrameP2Y' )
p1y = p1y + 75 -- It starts 75 pixels down from the top of the Bonus frame.
p2y = p2y + 75

t[#t+1] = LoadActor("life graph", PLAYER_1) .. {
	InitCommand = cmd(x,p1x;y,p1y);
};
t[#t+1] = LoadActor("life graph", PLAYER_2) .. {
	InitCommand = cmd(x,p2x;y,p2y);
};

t[#t+1] = LoadActor( THEME:GetPathB('','_standard decoration required'), "TimingDifficultyFrame", "TimingDifficultyFrame" );
t[#t+1] = LoadFont( Var "LoadingScreen", "TimingDifficulty" ) .. {
	InitCommand=function(self) self:name("TimingDifficulty"); self:settext(GetTimingDifficulty()); ActorUtil.LoadAllCommandsAndSetXY(self,Var "LoadingScreen"); end;
};

return t;