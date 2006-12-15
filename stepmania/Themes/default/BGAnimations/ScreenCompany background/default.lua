local t =
Def.ActorFrame {
	children = {
		LoadActor("white.png") .. {
			OnCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;stretchto,SCREEN_LEFT,SCREEN_TOP,SCREEN_RIGHT,SCREEN_BOTTOM)
		},
		LoadActor("shadow (32bpp).png") .. {
			OnCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;diffuse,0,0,0,1;addx,20;addy,6;linear,8;addx,-40)
		},
		LoadActor("stepmania (dither).png") .. {
			File="stepmania (dither).png",
			OnCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y),
		}
	}
}

return t
