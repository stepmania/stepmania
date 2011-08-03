local t = Def.ActorFrame {};

t[#t+1] = Def.ActorFrame {
  FOV=90;
  InitCommand=cmd(Center);
	Def.Quad {
		InitCommand=cmd(scaletoclipped,SCREEN_WIDTH,SCREEN_HEIGHT);
		OnCommand=cmd(diffuse,color("#FFCB05");diffusebottomedge,color("#F0BA00"));
	};
	Def.ActorFrame {
		LoadActor("_pattern") .. {
			InitCommand=cmd(rotationy,-12.25;rotationz,-30;rotationx,-20;zoomto,SCREEN_WIDTH*2,SCREEN_HEIGHT*2;customtexturerect,0,0,SCREEN_WIDTH*4/256,SCREEN_HEIGHT*4/256);
			OnCommand=cmd(texcoordvelocity,0.125,0.5;diffuse,color("#ffd400");diffusealpha,0.045;bob;effectmagnitude,0,0,35;effectperiod,4);
		};
	};
	LoadActor("_particleLoader") .. {
		InitCommand=cmd(x,-SCREEN_CENTER_X;y,-SCREEN_CENTER_Y);
	};
--[[ 	LoadActor("_particles") .. {
		InitCommand=cmd(x,-SCREEN_CENTER_X;y,-SCREEN_CENTER_Y);
	}; --]]

	Def.Quad {
		InitCommand=cmd(vertalign,top;scaletoclipped,SCREEN_WIDTH+1,80;y,-SCREEN_CENTER_Y+20;fadebottom,0.75);
		OnCommand=cmd(diffuse,color("#FFCB05"));
	};
	Def.Quad {
		InitCommand=cmd(vertalign,bottom;scaletoclipped,SCREEN_WIDTH+1,80;y,SCREEN_CENTER_Y-20;fadetop,0.75);
		OnCommand=cmd(diffuse,color("#FFCB05"));
	};
--[[ 	LoadActor("_pattern") .. {
		InitCommand=cmd(z,32;x,4;y,4;;rotationy,-12.25;rotationz,-30;rotationx,-20;zoomto,SCREEN_WIDTH*2,SCREEN_HEIGHT*2;customtexturerect,0,0,SCREEN_WIDTH*4/256,SCREEN_HEIGHT*4/256);
		OnCommand=cmd(texcoordvelocity,0.125,0.5;diffuse,Color("Black");diffusealpha,0.5);
	}; --]]
--[[ 	LoadActor("_grid") .. {
		InitCommand=cmd(customtexturerect,0,0,(SCREEN_WIDTH+1)/4,SCREEN_HEIGHT/4;SetTextureFiltering,true);
		OnCommand=cmd(zoomto,SCREEN_WIDTH+1,SCREEN_HEIGHT;diffuse,Color("Orange");diffuseshift;effecttiming,(1/8)*2,0,(7/8)*2,0;effectclock,'beatnooffset';
		effectcolor2,Color("Orange");effectcolor1,Color.Alpha(Color("Orange"),0.45);fadebottom,0.25;fadetop,0.25;croptop,48/480;cropbottom,48/480);
	}; --]]
	LoadActor("_bg top") .. {
		InitCommand=cmd(scaletoclipped,SCREEN_WIDTH+1,SCREEN_HEIGHT);
	};
};

return t;
