local font = LoadFont( "blaster" );
local frame = LoadActor( "_score frame" );
local text = GAMESTATE:GetPlayMode() == 'PlayMode_Oni' and "time" or "score";
return Def.ActorFrame {
	Def.ActorFrame {
		frame;
		font .. {
			Text = "player 1 " .. text;
			InitCommand = cmd(addy,14;addx,-20;zoom,0.4;shadowlength,0);
		};
		InitCommand = cmd(x,SCREEN_LEFT-100;y,SCREEN_CENTER_Y+202);
		OnCommand = cmd(linear,1;x,SCREEN_CENTER_X-208);
		OffCommand = cmd(linear,1;x,SCREEN_LEFT-100)
	};
	Def.ActorFrame {
		frame;
		font .. {
			Text = "player 2 " .. text;
			InitCommand = cmd(addy,14;addx,20;zoom,0.4;shadowlength,0);
		};
		InitCommand = cmd(x,SCREEN_RIGHT+100;y,SCREEN_CENTER_Y+202);
		OnCommand = cmd(linear,1;x,SCREEN_CENTER_X+210);
		OffCommand = cmd(linear,1;x,SCREEN_RIGHT+100)
	}
};
