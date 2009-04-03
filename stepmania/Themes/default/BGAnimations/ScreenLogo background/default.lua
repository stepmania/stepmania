return Def.ActorFrame {
	InitCommand=cmd(fov,90;);
	LoadActor( "bg sky" ) .. {
		InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y);
	};
	LoadActor( "bg rainbow" ) .. {
		InitCommand=cmd(x,SCREEN_LEFT;y,SCREEN_BOTTOM;horizalign,left;vertalign,bottom;blend,"BlendMode_Add";diffusealpha,0.4;);
	};
	LoadActor( "ring orange" ) .. {
		InitCommand=cmd(x,SCREEN_CENTER_X+50;y,SCREEN_CENTER_Y;diffusealpha,0.5;zoom,0.9;spin;rotationx,-20;rotationy,28;rotationz,20;effectmagnitude,0,0,5;);
	};
	LoadActor( "ring blue" ) .. {
		InitCommand=cmd(x,SCREEN_CENTER_X-100;y,SCREEN_CENTER_Y+50;diffusealpha,0.5;zoom,0.8;spin;rotationx,-60;rotationy,-20;rotationz,-50;effectmagnitude,0,0,-5;);
	};
};