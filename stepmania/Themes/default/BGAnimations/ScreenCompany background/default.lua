local t =
Def.ActorFrame {
	children = {
		LoadActor("white.png") .. {
			OnCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;stretchto,SCREEN_LEFT,SCREEN_TOP,SCREEN_RIGHT,SCREEN_BOTTOM)
		},
		LoadActor("stepmania (dither).png") .. {
			File="stepmania (dither).png",
			InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;diffusealpha,0),
			OnCommand=cmd(linear,1;diffusealpha,1),
		}
	}
}

return t
