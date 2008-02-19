return LoadActor( THEME:GetPathB("ScreenWithMenuElements2","overlay") ) .. {
	Def.ActorFrame {
		InitCommand=cmd(x,SCREEN_CENTER_X-200;y,SCREEN_BOTTOM-34;);
		LoadActor( "notify" ) .. {
			OnCommand=cmd();
		};
		LoadFont( "_venacti bold 30" ) .. {
			InitCommand=cmd(y,-15;shadowlength,0;diffuse,color("#000000");zoom,0.5;settext,"\"JOY4\" IS NOT MAPPED";skewx,-0.1;);
		};
	};
	Def.ActorFrame {
		InitCommand=cmd(x,SCREEN_CENTER_X+200;y,SCREEN_BOTTOM-34;);
		LoadActor( "notify" ) .. {
			OnCommand=cmd(zoomx,-1);
		};
		LoadFont( "_venacti bold 30" ) .. {
			InitCommand=cmd(y,-15;shadowlength,0;diffuse,color("#000000");zoom,0.5;settext,"P2: PRESS START TO JOIN\nTHIS IS 2 LINES OF TEXT";skewx,-0.1;);
		};
	};
};