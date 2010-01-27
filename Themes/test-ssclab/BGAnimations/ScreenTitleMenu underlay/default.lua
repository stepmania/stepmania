return Def.ActorFrame {
	FOV=90;
	LoadActor("_logo") .. {
		InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y);
		OnCommand=cmd(zoom,1.25;glow,color("1,1,1,1");decelerate,0.5;zoom,1;glow,color("1,1,1,0");glowshift;effecttiming,0.125,0,0.75,3);
		OffCommand=cmd(diffusealpha,0.5;glow,color("1,1,1,1");decelerate,0.25;glow,color("1,1,1,0");accelerate,0.125;rotationx,90;diffusealpha,0;);
	};
};