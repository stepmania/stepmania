return Def.ActorFrame {
	LoadActor("arrow") .. {
		InitCommand=cmd(y,8;shadowlengthx,0;shadowlengthy,2;);
	};
	LoadFont("Common normal") .. {
		InitCommand=cmd(y,-8;settext,"Barely";shadowlengthx,0;shadowlengthy,2;);
	};
};