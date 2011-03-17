local t = Def.ActorFrame {};
t[#t+1] = Def.Sprite {
	InitCommand=cmd(Center);
	BeginCommand=cmd(LoadFromCurrentSongBackground);
	OnCommand=cmd(diffusealpha,1;scaletoclipped,SCREEN_WIDTH,SCREEN_HEIGHT;linear,1;diffusealpha,0;);
};
return t;