return Def.ActorFrame {
	LoadActor( "top" ) .. {
		InitCommand=cmd();
	};
	Def.ActorFrame {
		InitCommand=cmd(x,-284;y,-6;);
		LoadActor( "bar" ) .. {
			InitCommand=cmd(horizalign,left;y,4);
		};
		LoadFont( "_sf sports night ns upright 52" ) .. {
			InitCommand=cmd(x,64;y,-7;horizalign,left;shadowlength,0;zoom,0.5;settext,"Select Style";skewx,-0.15);
		};
		LoadFont( "_venacti 24" ) .. {
			InitCommand=cmd(x,64;y,12;horizalign,left;shadowlength,0;zoom,0.5;settext,"I can't wait to see your next moves!");
		};
		LoadActor( "arrow" ) .. {
		};
		LoadActor( "ring" ) .. {
			InitCommand=cmd(x,-1.5);
		};
	};
};