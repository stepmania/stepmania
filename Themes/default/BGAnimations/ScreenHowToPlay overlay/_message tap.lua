return Def.ActorFrame{
	LoadActor("tapmessage") .. {
		InitCommand=cmd(x,SCREEN_CENTER_X+160;y,SCREEN_CENTER_Y+40;diffusealpha,0;);
		ShowCommand=cmd(linear,0;diffusealpha,1;sleep,2;linear,0;diffusealpha,0);
	};
};