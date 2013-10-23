local t = Def.ActorFrame {};

t[#t+1] = Def.ActorFrame {
  FOV=90;
  InitCommand=cmd(Center);
	Def.Quad {
		InitCommand=cmd(scaletoclipped,SCREEN_WIDTH,SCREEN_HEIGHT);
		OnCommand=cmd(diffuse,color("#FFCB05");diffusebottomedge,color("#F0BA00"));
	};
	Def.ActorFrame {
		InitCommand=cmd(hide_if,hideFancyElements;);
		LoadActor("_checkerboard") .. {
			InitCommand=cmd(rotationy,0;rotationz,0;rotationx,-90/4*3.5;zoomto,SCREEN_WIDTH*2,SCREEN_HEIGHT*2;customtexturerect,0,0,SCREEN_WIDTH*4/256,SCREEN_HEIGHT*4/256);
			OnCommand=cmd(texcoordvelocity,0,0.25;diffuse,color("#ffd400");fadetop,1);
		};
	};
	LoadActor("_particleLoader") .. {
		InitCommand=cmd(x,-SCREEN_CENTER_X;y,-SCREEN_CENTER_Y;hide_if,hideFancyElements;);
	};
--[[ 	LoadActor("_particles") .. {
		InitCommand=cmd(x,-SCREEN_CENTER_X;y,-SCREEN_CENTER_Y);
	}; --]]
--[[ 	LoadActor("_pattern") .. {
		InitCommand=cmd(z,32;x,4;y,4;;rotationy,-12.25;rotationz,-30;rotationx,-20;zoomto,SCREEN_WIDTH*2,SCREEN_HEIGHT*2;customtexturerect,0,0,SCREEN_WIDTH*4/256,SCREEN_HEIGHT*4/256);
		OnCommand=cmd(texcoordvelocity,0.125,0.5;diffuse,Color("Black");diffusealpha,0.5);
	}; --]]
	--[[ LoadActor("_grid") .. {
		InitCommand=cmd(customtexturerect,0,0,(SCREEN_WIDTH+1)/4,SCREEN_HEIGHT/4;SetTextureFiltering,true);
		OnCommand=cmd(zoomto,SCREEN_WIDTH+1,SCREEN_HEIGHT;diffuse,Color("Black");diffuseshift;effecttiming,(1/8)*2,0,(7/8)*2,0;effectclock,'beatnooffset';
		effectcolor2,Color("White");effectcolor1,Color("Black");fadebottom,0.25;fadetop,0.25;croptop,48/480;cropbottom,48/480;blend,Blend.Add;
		diffusealpha,0.155);
	}; --]]		
};

return t;
