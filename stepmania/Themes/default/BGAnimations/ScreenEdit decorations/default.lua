return Def.ActorFrame {
	Def.ActorFrame {
		Def.Quad { 
			InitCommand=cmd(horizalign,left;zoomtowidth,120;zoomtoheight,SCREEN_HEIGHT;diffuse,color("#000000");diffuserightedge,color("#00000011"););
			OnCommand=cmd();
		};	
		LoadFont("common normal") .. {
			InitCommand=cmd(x,60;y,-220;settext,ScreenString("Help");strokecolor,color("#00000077"););
		};
		InitCommand=cmd(x,SCREEN_LEFT;y,SCREEN_CENTER_Y;);
		EditCommand=cmd(playcommand,"Show");
		PlayingCommand=cmd(playcommand,"Hide");
		RecordCommand=cmd(playcommand,"Hide");
		RecordPausedCommand=cmd(playcommand,"Hide");
		ShowCommand=cmd(stoptweening;accelerate,0.25;x,SCREEN_LEFT;);
		HideCommand=cmd(stoptweening;accelerate,0.25;x,SCREEN_LEFT-192;);
	};
	Def.ActorFrame {
		Def.Quad { 
			InitCommand=cmd(horizalign,right;zoomtowidth,120;zoomtoheight,SCREEN_HEIGHT;diffuse,color("#000000");diffuseleftedge,color("#00000011"););
			OnCommand=cmd();
		};	
		LoadFont("common normal") .. {
			InitCommand=cmd(x,-60;y,-220;settext,ScreenString("Info");strokecolor,color("#00000077"););
		};
		InitCommand=cmd(x,SCREEN_RIGHT;y,SCREEN_CENTER_Y;);
		EditCommand=cmd(playcommand,"Show");
		PlayingCommand=cmd(playcommand,"Hide");
		RecordCommand=cmd(playcommand,"Hide");
		RecordPausedCommand=cmd(playcommand,"Hide");
		ShowCommand=cmd(stoptweening;accelerate,0.25;x,SCREEN_RIGHT;);
		HideCommand=cmd(stoptweening;accelerate,0.25;x,SCREEN_RIGHT+192;);
	};
};
