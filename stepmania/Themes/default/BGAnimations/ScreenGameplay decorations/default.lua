local t = LoadFallbackB();

t[#t+1] = StandardDecorationFromFile( "StageFrame", "StageFrame" );

t[#t+1] = LoadActor("_warning") .. {
	InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;
		vertalign,top;
		wag;effectmagnitude,0,0,10;effectperiod,2;
	);
	OnCommand=cmd(diffusealpha,0);
	ShowDangerAllMessageCommand=cmd(stoptweening;accelerate,0.3;diffusealpha,1);
	HideDangerAllMessageCommand=cmd(stoptweening;accelerate,0.3;diffusealpha,0);
};
	
t[#t+1] = StandardDecorationFromFile( "LifeFrame", "LifeFrame" );
t[#t+1] = StandardDecorationFromFile( "ScoreFrame", "ScoreFrame" );
t[#t+1] = StandardDecorationFromFile( "LeftFrame", "LeftFrame" );
t[#t+1] = StandardDecorationFromFile( "RightFrame", "RightFrame" );


t[#t+1] = Def.ModIconRow {
	InitCommand=cmd(x,THEME:GetMetric(Var "LoadingScreen","LeftFrameX");y,THEME:GetMetric(Var "LoadingScreen","LeftFrameY");Load,"ModIconRowGameplayP1",PLAYER_1;player,PLAYER_1;);
	OnCommand=cmd(zoomy,0;linear,0.5;zoomy,1);
	OffCommand=cmd(linear,0.5;zoomy,0);
};
t[#t+1] = Def.ModIconRow {
	InitCommand=cmd(x,THEME:GetMetric(Var "LoadingScreen","RightFrameX");y,THEME:GetMetric(Var "LoadingScreen","RightFrameY");Load,"ModIconRowGameplayP2",PLAYER_2;player,PLAYER_2;);
	OnCommand=cmd(zoomy,0;linear,0.5;zoomy,1);
	OffCommand=cmd(linear,0.5;zoomy,0);
};


return t;
