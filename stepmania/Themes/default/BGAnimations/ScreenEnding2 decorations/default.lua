local t = LoadFallbackB();
t[#t+1] = LoadActor("bg") .. {
	InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;);
};
t[#t+1] = LoadActor( "song background scroller" ) .. {
	InitCommand=cmd(x,SCREEN_CENTER_X-174;y,SCREEN_CENTER_Y;);
};
t[#t+1] = LoadActor( "credits" ) .. {
	InitCommand=cmd(x,SCREEN_CENTER_X+114;y,SCREEN_CENTER_Y;);
};
t[#t+1] = LoadActor("team") .. {
	InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y-30;);
	OnCommand=cmd(sleep,1;decelerate,0.7;addx,274;rotationz,90;);
};


return t;
