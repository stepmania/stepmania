local OnCommand = cmd(diffusealpha,0;linear,0.5;diffusealpha,1)
local OffCommand = cmd(linear,0.5;diffusealpha,0)
local t = Def.ActorFrame {
	LoadActor( '_difficulty frame p1' ) .. {
		InitCommand = cmd(player,PLAYER_1;x,SCREEN_CENTER_X-275;y,SCREEN_CENTER_Y-15);
		OnCommand = OnCommand;
		OffCommand = OffCommand;
	};
	LoadActor( '_difficulty frame p2' ) .. {
		InitCommand = cmd(player,PLAYER_2;x,SCREEN_CENTER_X-82;y,SCREEN_CENTER_Y-15);
		OnCommand = OnCommand;
		OffCommand = OffCommand;
	};
}
return t
