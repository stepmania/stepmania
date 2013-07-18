return Def.ActorFrame{
	LoadActor("jumpmessage") .. {
		InitCommand=cmd(x,SCREEN_CENTER_X+160;y,SCREEN_CENTER_Y+40;diffusealpha,0;);
		ShowCommand=cmd(linear,0;diffusealpha,1;sleep,1.7;linear,0;diffusealpha,0);
	};
};