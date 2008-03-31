local t = LoadFallbackB( "decorations" );

t[#t+1] = LoadActor( THEME:GetPathB('','_standard decoration required'), "StageFrame", "StageFrame" );

t[#t+1] = LoadActor("_warning") .. {
		InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;
			vertalign,top;
			wag;effectmagnitude,0,0,10;effectperiod,2;
		);
		OnCommand=cmd(diffusealpha,0);
		ShowDangerAllMessageCommand=cmd(stoptweening;accelerate,0.3;diffusealpha,1);
		HideDangerAllMessageCommand=cmd(stoptweening;accelerate,0.3;diffusealpha,0);
	};
	
t[#t+1] = LoadActor( THEME:GetPathB('','_standard decoration required'), "LifeFrame", "LifeFrame" );
t[#t+1] = LoadActor( THEME:GetPathB('','_standard decoration required'), "ScoreFrame", "ScoreFrame" );
t[#t+1] = LoadActor( THEME:GetPathB('','_standard decoration required'), "LeftFrame", "LeftFrame" );
t[#t+1] = LoadActor( THEME:GetPathB('','_standard decoration required'), "RightFrame", "RightFrame" );


t[#t+1] = Def.OptionIconRow {
		Condition=GAMESTATE:IsHumanPlayer(PLAYER_1);
		InitCommand=cmd(z,SCREEN_CENTER_X-200;y,SCREEN_CENTER_Y;Load,"OptionIconRowGameplay",PLAYER_1);
		OnCommand=cmd(zoomy,0;linear,0.5;zoomy,1);
		OffCommand=cmd(linear,0.5;zoomy,0);
	};
t[#t+1] = Def.OptionIconRow {
		Condition=GAMESTATE:IsHumanPlayer(PLAYER_2);
		InitCommand=cmd(z,SCREEN_CENTER_X+200;y,SCREEN_CENTER_Y;Load,"OptionIconRowGameplay",PLAYER_2;);
		OnCommand=cmd(zoomy,0;linear,0.5;zoomy,1);
		OffCommand=cmd(linear,0.5;zoomy,0);
	};


return t;
