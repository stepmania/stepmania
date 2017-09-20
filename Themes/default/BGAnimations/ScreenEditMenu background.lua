local t = Def.ActorFrame {};

t[#t+1] = Def.ActorFrame {
  InitCommand=cmd(Center);
	Def.Quad {
		InitCommand=cmd(scaletoclipped,SCREEN_WIDTH,SCREEN_HEIGHT);
		OnCommand=cmd(diffuse,color("#947B7E");diffusebottomedge,color("#D698A0"));
	};
};

t[#t+1] = Def.ActorFrame {
	Def.Quad {
		InitCommand=cmd(horizalign,left;zoomto,SCREEN_WIDTH/2,SCREEN_HEIGHT*0.78;x,SCREEN_LEFT;y,SCREEN_CENTER_Y+21;);
		OnCommand=cmd(diffuse,color("#FFFFFF");diffusealpha,0.25);
	};
};

return t;
