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

t[#t+1] = Def.OptionIconRow {
		InitCommand=cmd(x,SCREEN_CENTER_X-316;y,SCREEN_CENTER_Y-130;Load,"OptionIconRowEvaluation",PLAYER_1,player,PLAYER_1);
		OnCommand=cmd(zoomy,0;linear,0.5;zoomy,1);
		OffCommand=cmd(linear,0.5;zoomy,0);
	};
t[#t+1] = Def.OptionIconRow {
		InitCommand=cmd(x,SCREEN_CENTER_X+316;y,SCREEN_CENTER_Y-130;Load,"OptionIconRowEvaluation",PLAYER_2;player,PLAYER_2);
		OnCommand=cmd(zoomy,0;linear,0.5;zoomy,1);
		OffCommand=cmd(linear,0.5;zoomy,0);
	};

t[#t+1] = Def.DifficultyDisplay {
		InitCommand=cmd(x,SCREEN_CENTER_X-230;y,SCREEN_CENTER_Y+158;Load,"DifficultyDisplayEvaluation",PLAYER_1;SetFromGameState,PLAYER_1;player,PLAYER_1;);
		OnCommand=cmd(zoomy,0;linear,0.5;zoomy,1);
		OffCommand=cmd(linear,0.5;zoomy,0);
	};
t[#t+1] = Def.DifficultyDisplay {
		InitCommand=cmd(x,SCREEN_CENTER_X+230;y,SCREEN_CENTER_Y+158;Load,"DifficultyDisplayEvaluation",PLAYER_2;SetFromGameState,PLAYER_2;player,PLAYER_2;);
		OnCommand=cmd(zoomy,0;linear,0.5;zoomy,1);
		OffCommand=cmd(linear,0.5;zoomy,0);
	};

return t;