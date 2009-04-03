return Def.ActorFrame {
	LoadActor( THEME:GetPathG(Var "LoadingScreen","GameLogo "..GAMESTATE:GetCurrentGame():GetName()) ) .. {
		InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;);
		OnCommand=cmd(zoomy,0;bounceend,0.5;zoomy,1;glowshift;effectperiod,2.5;effectcolor1,1,1,1,0.0;effectcolor2,1,1,1,0.5;);
	};
};