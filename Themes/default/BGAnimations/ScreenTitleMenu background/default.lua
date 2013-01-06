local t = Def.ActorFrame {};
t[#t+1] = Def.ActorFrame {
	FOV=90;
	InitCommand=cmd(Center);
	Def.Quad {
		InitCommand=cmd(scaletoclipped,SCREEN_WIDTH,SCREEN_HEIGHT);
		OnCommand=cmd(diffuse,color("#FFCB05");diffusebottomedge,color("#F0BA00"));
	};
	Def.ActorFrame {
		OnCommand=cmd(spin;effectmagnitude,0,0,10);
		Def.ActorFrame {
			InitCommand=cmd(rotationx,30;hide_if,hideFancyElements;);
			OnCommand=cmd(sleep,2;queuecommand,"Shift");
			ShiftCommand=cmd(
				smooth,1.25;z,256;
				sleep,2;
				smooth,1.25;z,0;
				sleep,2;queuecommand,"Shift"
			);
			FlipCommand=cmd(
				smooth,0.5;rotationy,180;
				sleep,2;smooth,0.5;rotationy,360;
				sleep,1;rotationy,0;sleep,1;
				queuecommand,"Flip"
			);
			LoadActor(THEME:GetPathB("ScreenWithMenuElements","background/_checkerboard")) .. {
				InitCommand=cmd(zoomto,SCREEN_WIDTH*2,SCREEN_HEIGHT*2;customtexturerect,0,0,SCREEN_WIDTH*4/256,SCREEN_HEIGHT*4/256);
				OnCommand=cmd(texcoordvelocity,0,0.5;diffuse,color("#ffd400");diffusealpha,0.5;fadetop,1;fadebottom,1);
			};
		};
	};
};
return t;