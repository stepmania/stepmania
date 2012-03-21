local t = Def.ActorFrame {};
t[#t+1] = Def.Sprite {
	InitCommand=cmd(Center);
	BeginCommand=cmd(LoadFromCurrentSongBackground);
	OnCommand=cmd(diffusealpha,1;scale_or_crop_background;linear,1;diffusealpha,0;);
};
return t;