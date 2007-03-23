local children = {
	LoadActor( THEME:GetPathS("", "_swoosh normal") ) .. {
		StartTransitioningCommand=cmd(play);
	};
	Def.Actor { OnCommand=cmd(sleep,2); };
	LoadSongBackground() .. {
		Condition=not STATSMAN:GetCurStageStats():AllFailed();
		InitCommand=cmd(scaletoclipped,SCREEN_WIDTH,SCREEN_HEIGHT;x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y);
		OnCommand=cmd(diffusealpha,1;linear,0.4;diffusealpha,0);
	};
	LoadActor( "_black.png" ) .. {
		Condition=STATSMAN:GetCurStageStats():AllFailed();
		InitCommand=cmd(stretchto,SCREEN_LEFT,SCREEN_TOP,SCREEN_RIGHT,SCREEN_BOTTOM);
		OnCommand=cmd(diffusealpha,1;linear,0.2;diffusealpha,0);
	};
	-- Cleared
	Def.ActorFrame {
		Condition=GAMESTATE:GetPlayMode() ~= PLAY_MODE_BATTLE and GAMESTATE:GetPlayMode() ~= PLAY_MODE_RAVE;
		children = {
			LoadActor( "ScreenGameplay out/_extra1.png" ) .. {
				Condition=GAMESTATE:IsFinalStage();
				InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y);
				StartTransitioningCommand=cmd(hide_if,not GAMESTATE:HasEarnedExtraStage());
				OnCommand=cmd(diffusealpha,0.4;linear,0.2;diffusealpha,0);
			};
			LoadActor( "ScreenGameplay out/_extra2.png" ) .. {
				Condition=GAMESTATE:IsExtraStage();
				InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y);
				StartTransitioningCommand=cmd(hide_if,not GAMESTATE:HasEarnedExtraStage());
				OnCommand=cmd(diffusealpha,0.4;linear,0.2;diffusealpha,0);
			};
			LoadActor( "ScreenGameplay failed/failed.png" ) .. {
				Condition=STATSMAN:GetCurStageStats():AllFailed();
				InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y);
				OnCommand=cmd(diffusealpha,0.4;linear,0.2;diffusealpha,0);
			};
			LoadActor( "ScreenGameplay out/cleared.png" ) .. {
				Condition=not STATSMAN:GetCurStageStats():AllFailed();
				InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y);
				StartTransitioningCommand=cmd(hide_if,GAMESTATE:HasEarnedExtraStage());
				OnCommand=cmd(diffusealpha,1;linear,0.2;diffusealpha,0);
			};
		};
	};
	-- Winner
	Def.ActorFrame {
		Condition=GAMESTATE:GetPlayMode() == PLAY_MODE_BATTLE or GAMESTATE:GetPlayMode() == PLAY_MODE_RAVE;
		children = {
			LoadActor( "ScreenGameplay out/p1 win.png" ) .. {
				InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y);
				StartTransitioningCommand=cmd(hide_if,not GAMESTATE:IsWinner(PLAYER_1));
				OnCommand=cmd(diffusealpha,1;linear,0.2;diffusealpha,0);
			};
			LoadActor("ScreenGameplay out/p2 win.png" ) .. {
				InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y);
				StartTransitioningCommand=cmd(hide_if,not GAMESTATE:IsWinner(PLAYER_2));
				OnCommand=cmd(diffusealpha,1;linear,0.2;diffusealpha,0);
			};
			LoadActor( "ScreenGameplay out/draw.png" ) .. {
				InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y);
				StartTransitioningCommand=cmd(hide_if,not GAMESTATE:IsDraw());
				OnCommand=cmd(diffusealpha,1;linear,0.2;diffusealpha,0);
			};
		};
	};

};
return WrapInActorFrame( children );
