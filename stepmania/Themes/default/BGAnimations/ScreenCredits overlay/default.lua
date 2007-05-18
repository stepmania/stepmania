local t = Def.ActorFrame {
	LoadActor( "song background scroller" ) .. {
		InitCommand = cmd(x,SCREEN_CENTER_X-160;y,SCREEN_CENTER_Y);
	};
	LoadActor( "credits" ) .. {
		InitCommand = cmd(x,SCREEN_CENTER_X+160;y,SCREEN_CENTER_Y);
	};
}

return t;
