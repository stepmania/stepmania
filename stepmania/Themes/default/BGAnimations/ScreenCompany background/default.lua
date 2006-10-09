local t =
{
	Type = "ActorFrame",
	children =
	{
		{
			File="white.png",
			OnCommand="x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;stretchto,SCREEN_LEFT,SCREEN_TOP,SCREEN_RIGHT,SCREEN_BOTTOM"
		},
		{
			File="shadow (32bpp).png",
			OnCommand="x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;diffuse,0,0,0,1;addx,+20;addy,6;linear,8;addx,-40"
		},
		{
			File="stepmania (dither).png",
			OnCommand="x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y"
		}
	}
		
}

return t
