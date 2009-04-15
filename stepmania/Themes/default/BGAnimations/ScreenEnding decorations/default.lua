return Def.ActorFrame {
	Def.ActorFrame {
		InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y+200;draworder,1;);
		LoadActor("bar") .. {
		};
		LoadFont("_venacti Bold 15px") .. {
			InitCommand=cmd(settext,"Thank you for playing!";horizalign,left;x,-280;shadowlength,0;strokecolor,color("#000000"););
		};
	};
};