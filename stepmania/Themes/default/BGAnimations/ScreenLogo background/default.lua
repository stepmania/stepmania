return Def.ActorFrame {
	LoadActor( "bg sky" ) .. {
		InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y);
	};
	LoadActor( "bg ring" ) .. {
		InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y);
	};
	LoadActor( "bg rainbow" ) .. {
		InitCommand=cmd(x,SCREEN_LEFT;y,SCREEN_BOTTOM;horizalign,left;vertalign,bottom;);
	};
	LoadActor( "cursor frame" ) .. {
		InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;);
	};
	LoadActor( "help pane" ) .. {
		InitCommand=cmd(x,SCREEN_CENTER_X+200;y,SCREEN_BOTTOM;vertalign,bottom;);
	};
	LoadActor( "settings pane" ) .. {
		InitCommand=cmd(x,SCREEN_CENTER_X-200;y,SCREEN_TOP;vertalign,top;);
	};
}