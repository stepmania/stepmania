local t = Def.ActorFrame {
	LoadActor("../_black.png") .. {
		OnCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;zoomtowidth,SCREEN_WIDTH;zoomtoheight,120;diffusealpha,0.7);
	};
	LoadActor("demonstration.png") .. {
		OnCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;diffuseblink;effectperiod,1);
	};
};

return t;
