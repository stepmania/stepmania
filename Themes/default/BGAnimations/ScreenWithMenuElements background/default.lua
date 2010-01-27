local t = Def.ActorFrame {};

t[#t+1] = Def.ActorFrame {
  InitCommand=cmd(Center);
	Def.Quad {
		InitCommand=cmd(scaletoclipped,SCREEN_WIDTH,SCREEN_HEIGHT);
		OnCommand=cmd(diffuse,color("#FFCB05");diffusebottomedge,color("#F0BA00"));
	};
	LoadActor("_bg top") .. {
		InitCommand=cmd(scaletoclipped,SCREEN_WIDTH,SCREEN_HEIGHT);
	};
};

return t;
