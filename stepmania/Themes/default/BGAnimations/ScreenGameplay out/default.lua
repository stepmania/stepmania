local children = {
	LoadActor(GetSongBackground()) .. {
		InitCommand=cmd(scaletoclipped,SCREEN_RIGHT,SCREEN_BOTTOM;x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y);
		OnCommand=cmd(diffusealpha,0;sleep,0.5;linear,0.5;diffusealpha,1);
	};

	-- Cleared
	Def.ActorFrame {
		Condition=GAMESTATE:GetPlayMode() ~= PLAY_MODE_BATTLE and GAMESTATE:GetPlayMode() ~= PLAY_MODE_RAVE and not GAMESTATE:IsDemonstration();
		children = {
			LoadActor("extra1") .. {
				Condition=GAMESTATE:IsFinalStage();
				StartTransitioningCommand=cmd(hide_if,not GAMESTATE:HasEarnedExtraStage());
			};
			LoadActor("extra2") .. {
				Condition=GAMESTATE:IsExtraStage();
				StartTransitioningCommand=cmd(hide_if,not GAMESTATE:HasEarnedExtraStage());
			};
			LoadActor("cleared") .. {
				InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y);
				StartTransitioningCommand=cmd(hide_if,GAMESTATE:HasEarnedExtraStage());
				OnCommand=cmd(diffusealpha,0;sleep,1;linear,0.5;diffusealpha,1;sleep,2);
			};
		};
	};
	-- Winner
	Def.ActorFrame {
		Condition=GAMESTATE:GetPlayMode() == PLAY_MODE_BATTLE or GAMESTATE:GetPlayMode() == PLAY_MODE_RAVE;
		children = {
			LoadActor("p1 win") .. {
				InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y);
				StartTransitioningCommand=cmd(hide_if,not GAMESTATE:IsWinner(PLAYER_1));
				OnCommand=cmd(diffusealpha,0;sleep,1.5;linear,0.5;diffusealpha,1;sleep,2);
			};
			LoadActor("p2 win") .. {
				InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y);
				StartTransitioningCommand=cmd(hide_if,not GAMESTATE:IsWinner(PLAYER_2));
				OnCommand=cmd(diffusealpha,0;sleep,1.5;linear,0.5;diffusealpha,1;sleep,2);
			};
			LoadActor("draw") .. {
				InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y);
				StartTransitioningCommand=cmd(hide_if,not GAMESTATE:IsDraw());
				OnCommand=cmd(diffusealpha,0;sleep,1.5;linear,0.5;diffusealpha,1;sleep,2);
			};
		};
	};

	-- Demonstration
	Def.ActorFrame {
		Condition=GAMESTATE:IsDemonstration();
		children = {
			Def.Quad {
				InitCommand=cmd(stretchto,SCREEN_LEFT,SCREEN_TOP,SCREEN_RIGHT,SCREEN_BOTTOM;diffuse,color("0,0,0,1"));
				OnCommand=cmd(sleep,1.5;diffusealpha,0);
			};
		};
	};
};
return Def.ActorFrame { children = children };
