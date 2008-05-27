return Def.ActorFrame {
	Def.ActorFrame {
		InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;);
		LoadActor("frame") .. {
			InitCommand=cmd();
		};
		LoadFont("_terminator two 60") .. {
			InitCommand=cmd(y,-150;settext,ScreenString("Warning");diffuse,color("#fef500");strokecolor,color("#0167d2");shadowlength,0);
		};
		LoadFont("_venacti bold 30") .. {
			InitCommand=cmd(y,20;settext,ScreenString("BodyText");wrapwidthpixels,1000;strokecolor,color("#0167d2");shadowlength,0);
		};
	};
};