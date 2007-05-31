return Def.ActorFrame{
	-- timer background
	LoadActor(THEME:GetPathB("","_frame 3x1"), "headers", 48)..{
		--InitCommand=cmd( x,SCREEN_CENTER_X+(SCREEN_WIDTH/5.35) );
		InitCommand=cmd( x,SCREEN_RIGHT-(SCREEN_WIDTH/3.1825) );
	};
};