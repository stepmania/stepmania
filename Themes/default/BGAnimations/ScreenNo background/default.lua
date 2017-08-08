local t = Def.ActorFrame {};

t[#t+1] = Def.ActorFrame {
  FOV=90;
  InitCommand=cmd(Center);
	Def.Quad {
		InitCommand=cmd(scaletoclipped,SCREEN_WIDTH,SCREEN_HEIGHT);
		OnCommand=cmd(diffuse,color("#fdbe28");diffusebottomedge,color("#f67849"));
	};
	Def.ActorFrame {
    InitCommand=cmd(diffusealpha,0.2;);
		LoadActor("_checkerboard") .. {
			InitCommand=cmd(rotationy,0;rotationz,0;rotationx,-90/4*3.5;zoomto,SCREEN_WIDTH*2,SCREEN_HEIGHT*2;customtexturerect,0,0,SCREEN_WIDTH*4/256,SCREEN_HEIGHT*4/256);
			OnCommand=cmd(texcoordvelocity,0,0.25;diffuse,color("#ffffff");fadetop,1);
		};
	};
};

return t;
