local t = Def.ActorFrame {
	LoadActor( "toasty" ) .. {
		OnCommand = cmd(horizalign,left;vertalign,bottom;x,SCREEN_WIDTH;y,SCREEN_HEIGHT;decelerate,0.3;x,468;sleep,1.0;accelerate,0.3;x,SCREEN_WIDTH);
	};
	LoadActor( THEME:GetPathS("", "_toasty") ) .. {
		StartTransitioningCommand = cmd(play);
	};
}
return t
