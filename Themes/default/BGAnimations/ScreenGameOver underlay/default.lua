local timer_seconds = THEME:GetMetric(Var "LoadingScreen","TimerSeconds");

return Def.ActorFrame {
	InitCommand=cmd(Center),
	-- Fade
	Def.ActorFrame {
		InitCommand=cmd();
		Def.Quad {
			InitCommand=cmd(scaletoclipped,SCREEN_WIDTH,SCREEN_HEIGHT),
			OnCommand=cmd(diffuse,Color.Black;diffusealpha,0;linear,0.5;diffusealpha,0.25;
						sleep,timer_seconds/2;  
						linear,timer_seconds/2-0.5;diffusealpha,0.8),
		},
	},
	
	Def.Quad {
		InitCommand=cmd(zoomto,SCREEN_WIDTH,SCREEN_HEIGHT),
		OnCommand=cmd(diffuse,color("#000000")),
	},	
	
	LoadActor("_diamond") .. {
		InitCommand=cmd(diffusealpha,0);
		OnCommand=cmd(rotationz,0;zoom,1;sleep,1;diffusealpha,1;linear,3;zoom,1;diffusealpha,0);
	};
	
	LoadActor("_sound") .. {
		OnCommand=cmd(queuecommand,"Sound");
		SoundCommand=cmd(play);
	};	
	
	LoadActor(THEME:GetPathG("ScreenGameOver","gameover"))..{
		OnCommand=cmd(zoomx,1.1;diffusealpha,0;sleep,1;decelerate,0.6;diffusealpha,1;zoomx,1);
	},
	
	LoadFont("Common Condensed")..{
		Text=ScreenString("Play again soon!");
		InitCommand=cmd(y,68;shadowlength,2);
		OnCommand=cmd(diffusealpha,0;sleep,3;linear,0.3;diffusealpha,1);
	},
}
