return Def.ActorFrame {
	Def.ActorFrame {
		InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;);
		LoadActor("frame") .. {
			InitCommand=cmd();
		};
		LoadFont("_terminator two 40px") .. {
			InitCommand=cmd(y,-136;uppercase,true;settext,ScreenString("Warning");diffuse,color("#fef500");strokecolor,color("#0167d2");shadowlength,0;);
		};
		LoadFont("_venacti Bold 15px") .. {
			InitCommand=cmd(y,26;uppercase,true;settext,ScreenString("BodyText");wrapwidthpixels,500;strokecolor,color("#0167d2");shadowlength,0;vertspacing,2;);
		};
	};
};