local font = LoadFont( "_zeroesthree" );
local frame = LoadActor( "_score frame" );
local text = GAMESTATE:GetPlayMode() == 'PlayMode_Oni' and "time" or "score";
return Def.ActorFrame {
	Def.ActorFrame {
		frame;
		font .. {
			Text = "player 1 " .. text;
			InitCommand = cmd(horizalign,left;addy,16;addx,-80;zoom,0.75;shadowlength,0);
		};
		InitCommand = cmd(x,SCREEN_LEFT-100;y,SCREEN_CENTER_Y+202);
		OnCommand = cmd(linear,1;x,SCREEN_CENTER_X-208);
		OffCommand = cmd(linear,1;x,SCREEN_LEFT-100);
		Condition=GAMESTATE:IsSideJoined(PLAYER_1) == true or GAMESTATE:GetPlayMode() == 'PlayMode_Rave';
	};
	Def.ActorFrame {
		frame;
		font .. {
			Text = "player 2 " .. text;
			InitCommand = cmd(horizalign,right;addy,16;addx,80;zoom,0.75;shadowlength,0);
		};
		InitCommand = cmd(x,SCREEN_RIGHT+100;y,SCREEN_CENTER_Y+202);
		OnCommand = cmd(linear,1;x,SCREEN_CENTER_X+210);
		OffCommand = cmd(linear,1;x,SCREEN_RIGHT+100);
		Condition=GAMESTATE:IsSideJoined(PLAYER_2) == true or GAMESTATE:GetPlayMode() == 'PlayMode_Rave';
	}
};
