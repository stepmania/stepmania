local t = Def.ActorFrame {};

t[#t+1] = Def.ActorFrame {
  InitCommand=cmd(Center);
	Def.Quad {
		InitCommand=cmd(scaletoclipped,SCREEN_WIDTH+1,SCREEN_HEIGHT);
		OnCommand=cmd(diffuse,color("#FFCB05");diffusebottomedge,color("#F0BA00"));
	};
	LoadActor("_grid") .. {
		InitCommand=cmd(customtexturerect,0,0,(SCREEN_WIDTH+1)/4,SCREEN_HEIGHT/4;SetTextureFiltering,true);
		OnCommand=cmd(zoomto,SCREEN_WIDTH+1,SCREEN_HEIGHT;diffuse,Color("Orange");diffuseshift;effecttiming,(1/8)*2,0,(7/8)*2,0;effectclock,'beatnooffset';
		effectcolor2,Color("Orange");effectcolor1,Colors.Alpha(Color("Orange"),0.45);fadebottom,0.25;fadetop,0.25;croptop,48/480;cropbottom,48/480);
	};
	LoadActor("_bg top") .. {
		InitCommand=cmd(scaletoclipped,SCREEN_WIDTH+1,SCREEN_HEIGHT);
	};
};

return t;
