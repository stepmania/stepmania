function GraphDisplay( pn )
	local t = Def.ActorFrame {
		Def.GraphDisplay {
			InitCommand=cmd(Load,"GraphDisplay";);
			BeginCommand=function(self)
				local screen = SCREENMAN:GetTopScreen();
				local ss = screen:GetStageStats();
				self:Set( ss, ss:GetPlayerStageStats(pn) );
			end,
		};
	};
	return t;
end

function ComboGraph( pn )
	local t = Def.ActorFrame {
		Def.ComboGraph {
			InitCommand=cmd(Load,"ComboGraph";);
			BeginCommand=function(self)
				local screen = SCREENMAN:GetTopScreen();
				local ss = screen:GetStageStats();
				self:Set( ss, ss:GetPlayerStageStats(pn) );
			end,
		};
	};
	return t;
end


local t = LoadFallbackB();

if ShowStandardDecoration("GraphDisplay") then
	for pn in ivalues(PlayerNumber) do
		t[#t+1] = StandardDecorationFromTable( "GraphDisplay" .. PlayerNumber:ToString()[pn], GraphDisplay(pn) );
	end
end

if ShowStandardDecoration("ComboGraph") then
	for pn in ivalues(PlayerNumber) do
		t[#t+1] = StandardDecorationFromTable( "ComboGraph" .. PlayerNumber:ToString()[pn], ComboGraph(pn) );
	end
end

t[#t+1] = ComboGraph(PLAYER_1) .. {
	InitCommand = cmd(x,SCREEN_CENTER_X-222;y,SCREEN_CENTER_Y+10;draworder,1;);
};
t[#t+1] = ComboGraph(PLAYER_2) .. {
	InitCommand = cmd(x,SCREEN_CENTER_X+222;y,SCREEN_CENTER_Y+10;draworder,1;);
};

t[#t+1] = StandardDecorationFromFile( "TimingDifficultyFrame", "TimingDifficultyFrame" );
t[#t+1] = LoadFont( Var "LoadingScreen", "TimingDifficultyNumber" ) .. {
	InitCommand=function(self) self:name("TimingDifficultyNumber"); self:settext(GetTimingDifficulty()); ActorUtil.LoadAllCommandsAndSetXY(self,Var "LoadingScreen"); end;
};

t[#t+1] = Def.ModIconRow {
		InitCommand=cmd(x,SCREEN_CENTER_X-316;y,SCREEN_CENTER_Y-130;draworder,1;Load,"ModIconRowEvaluationP1",PLAYER_1,player,PLAYER_1);
		OnCommand=cmd(zoomy,0;linear,0.5;zoomy,1);
		OffCommand=cmd(linear,0.5;zoomy,0);
	};
t[#t+1] = Def.ModIconRow {
		InitCommand=cmd(x,SCREEN_CENTER_X+316;y,SCREEN_CENTER_Y-130;draworder,1;Load,"ModIconRowEvaluationP2",PLAYER_2;player,PLAYER_2);
		OnCommand=cmd(zoomy,0;linear,0.5;zoomy,1);
		OffCommand=cmd(linear,0.5;zoomy,0);
	};

t[#t+1] = Def.StepsDisplay {
		InitCommand=cmd(x,SCREEN_CENTER_X-230;y,SCREEN_CENTER_Y+158;Load,"StepsDisplayEvaluation",PLAYER_1;SetFromGameState,PLAYER_1;player,PLAYER_1;);
		OnCommand=cmd(zoomy,0;linear,0.5;zoomy,1);
		OffCommand=cmd(linear,0.5;zoomy,0);
	};
t[#t+1] = Def.StepsDisplay {
		InitCommand=cmd(x,SCREEN_CENTER_X+230;y,SCREEN_CENTER_Y+158;Load,"StepsDisplayEvaluation",PLAYER_2;SetFromGameState,PLAYER_2;player,PLAYER_2;);
		OnCommand=cmd(zoomy,0;linear,0.5;zoomy,1);
		OffCommand=cmd(linear,0.5;zoomy,0);
	};

return t;