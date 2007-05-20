local t = Def.ActorFrame {
	LoadFont("blaster") .. {
		Text="CAUTION!";
		InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_TOP+60;diffuse,1,0,0,1;zoom,1.5;diffusebottomedge,0.25,0,0,1;shadowlength,0);
	};
	
	LoadFont("_zeroesthree") .. {
		Text="For safety purposes, play with the lights on\nand keep the dance platform a safe distance\nfrom your TV or monitor as well as any other\nfurniture at all times.\nDo not allow pets, children, or other spectators\non the dance platform during gameplay.\nBe aware of the platform\'s position at all times.";
		InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;horizalign,center;shadowlength,0);
	};
};

return t;
