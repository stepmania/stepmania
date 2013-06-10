local t = LoadFallbackB();
t[#t+1] = LoadActor("bg") .. {
	InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;);
};
t[#t+1] = LoadActor( "picture scroller" ) .. {
	InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y-20;);
};


return t;
