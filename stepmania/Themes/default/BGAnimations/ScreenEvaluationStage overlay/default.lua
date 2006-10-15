return Def.ActorFrame {
	children = {
		LoadActor("life graph")(PLAYER_1) .. {
			InitCommand = cmd(x,SCREEN_LEFT+96;y,SCREEN_CENTER_Y+91),
		},
		LoadActor("life graph")(PLAYER_2) .. {
			InitCommand = cmd(x,SCREEN_LEFT-96;y,SCREEN_CENTER_Y+91),
		},
	}
}
