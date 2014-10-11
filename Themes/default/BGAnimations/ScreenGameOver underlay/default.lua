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
		-- Warning Fade
		Def.Quad {
			InitCommand=cmd(y,-4;scaletoclipped,SCREEN_WIDTH,164),
			OnCommand=cmd(diffuse,Color.Black;diffusealpha,0.5;
						  linear,timer_seconds;zoomtoheight,164*0.75),
		}
	},
	--
	LoadActor(THEME:GetPathG("ScreenGameOver","gameover"))..{
		InitCommand=cmd(y,-16;shadowlength,2)
	},
	LoadFont("Common Normal")..{
		Text=ScreenString("Play again soon!"),
		InitCommand=cmd(y,36;shadowlength,2)
	}
}
