return Def.ActorFrame {
	LoadActor("arrow") .. {
		InitCommand=cmd(y,-16;);
	};
	LoadFont("Common normal") .. {
		InitCommand=cmd(y,-32;settext,"Barely";);
	};
};