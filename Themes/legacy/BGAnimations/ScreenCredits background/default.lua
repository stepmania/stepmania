local scrolltime = 95;

local t = Def.ActorFrame {
	InitCommand=cmd(Center);
	LoadActor("_space")..{
		InitCommand=cmd(y,-SCREEN_HEIGHT*1.5;fadebottom,0.125;fadetop,0.25);
		OnCommand=cmd(linear,scrolltime;addy,SCREEN_HEIGHT*1.5825);
	};
	Def.Quad {
		InitCommand=cmd(scaletoclipped,SCREEN_WIDTH+1,SCREEN_HEIGHT);
		OnCommand=cmd(diffusecolor,color("#FFCB05");diffusebottomedge,color("#F0BA00");diffusealpha,0.45);
	};
	LoadActor("_grid")..{
		InitCommand=cmd(customtexturerect,0,0,(SCREEN_WIDTH+1)/4,SCREEN_HEIGHT/4;SetTextureFiltering,true);
		OnCommand=cmd(zoomto,SCREEN_WIDTH+1,SCREEN_HEIGHT;diffuse,Color("Black");diffuseshift;effecttiming,(1/8)*4,0,(7/8)*4,0;effectclock,'beatnooffset';
		effectcolor2,Color("Black");effectcolor1,Color.Alpha(Color("Black"),0.45);fadebottom,0.25;fadetop,0.25;croptop,48/480;cropbottom,48/480;diffusealpha,0.345);
	};
};

return t