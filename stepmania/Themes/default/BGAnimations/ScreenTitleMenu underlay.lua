return Def.ActorFrame {
	LoadActor( THEME:GetPathB("ScreenWithMenuElements", "underlay") );
	LoadActor( THEME:GetPathG("ScreenTitleMenu", GAMESTATE:GetCurrentGame():GetName()) ) .. {
		OnCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y-40;zoomy,0;sleep,0.5;bounceend,0.5;zoomy,1;glowshift;effectperiod,2.5;effectcolor1,1,1,1,0.1;effectcolor2,1,1,1,0.3);
	};
};
