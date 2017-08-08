local t = Def.ActorFrame {};

t[#t+1] = Def.ActorFrame {
  FOV=90;
  InitCommand=cmd(Center);
	Def.Quad {
		InitCommand=cmd(scaletoclipped,SCREEN_WIDTH,SCREEN_HEIGHT);
		OnCommand=cmd(diffuse,ColorMidTone(color("#451A20"));diffusebottomedge,ColorMidTone(color("#5E2A30"));diffusealpha,0.9);
	};
	LoadActor (GetSongBackground()) .. {
		InitCommand=cmd(scaletoclipped,SCREEN_WIDTH,SCREEN_HEIGHT);
		OnCommand=cmd(diffusealpha,0.1;);
	};
};

return t;
