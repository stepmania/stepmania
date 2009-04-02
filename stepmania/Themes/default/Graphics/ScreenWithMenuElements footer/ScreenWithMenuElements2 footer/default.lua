return LoadActor( THEME:GetPathB("ScreenWithMenuElements","overlay") ) .. {
	Def.ActorFrame {
		InitCommand=cmd(x,-200;);
		LoadActor( "notify" ) .. {
			OnCommand=cmd();
		};
		LoadFont( "_venacti bold 15px" ) .. {
			InitCommand=cmd(y,-15;shadowlength,0;diffuse,color("#000000");settext,"\"JOY4\" IS NOT MAPPED";skewx,-0.1;);
		};
	};
	Def.ActorFrame {
		InitCommand=cmd(x,200;);
		LoadActor( "notify" ) .. {
			OnCommand=cmd(zoomx,-1);
		};
		LoadFont( "_venacti bold 15px" ) .. {
			InitCommand=cmd(y,-15;shadowlength,0;diffuse,color("#000000");settext,"P2: PRESS START TO JOIN\nTHIS IS 2 LINES OF TEXT";skewx,-0.1;);
		};
	};
};