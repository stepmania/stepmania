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


local t = LoadFallbackB( "decorations" );

t[#t+1] = GraphDisplay(PLAYER_1) .. {
	InitCommand = cmd(x,SCREEN_CENTER_X-224;y,SCREEN_CENTER_Y-50;draworder,1;);
};
t[#t+1] = GraphDisplay(PLAYER_2) .. {
	InitCommand = cmd(x,SCREEN_CENTER_X+224;y,SCREEN_CENTER_Y-50;draworder,1;);
};

t[#t+1] = ComboGraph(PLAYER_1) .. {
	InitCommand = cmd(x,SCREEN_CENTER_X-224;y,SCREEN_CENTER_Y-20;draworder,1;);
};
t[#t+1] = ComboGraph(PLAYER_2) .. {
	InitCommand = cmd(x,SCREEN_CENTER_X+224;y,SCREEN_CENTER_Y-20;draworder,1;);
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