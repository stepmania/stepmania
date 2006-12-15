local p1x = THEME:GetMetric( 'ScreenEvaluationStage', 'BonusFrameP1X' )
local p1y = THEME:GetMetric( 'ScreenEvaluationStage', 'BonusFrameP1Y' )
local p2x = THEME:GetMetric( 'ScreenEvaluationStage', 'BonusFrameP2X' )
local p2y = THEME:GetMetric( 'ScreenEvaluationStage', 'BonusFrameP2Y' )
p1y = p1y + 75 -- It starts 75 pixels down from the top of the Bonus frame.
p2y = p2y + 75

Trace( "(x,y) = (" .. p1x .. "," .. p1y .. ")" )
return Def.ActorFrame {
	children = {
		LoadActor("life graph", PLAYER_1) .. {
			InitCommand = cmd(x,p1x;y,p1y),
		},
		LoadActor("life graph", PLAYER_2) .. {
			InitCommand = cmd(x,p2x;y,p2y),
		},
	}
}
