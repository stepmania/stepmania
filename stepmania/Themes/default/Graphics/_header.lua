local Text = ...
assert( Text );

return Def.ActorFrame{
	-- main header background
	LoadActor(THEME:GetPathB("","_frame 3x1"), "headers", SCREEN_WIDTH/2.5);
	
	-- text
	LoadFont("", "_zeroesthree") ..{
		Text=THEME:GetString( 'Headers', Text );
		InitCommand=cmd(horizalign,left;shadowlength,0;x,-128);
		OnCommand=cmd(linear,0.25;diffusealpha,1);
		OffCommand=cmd(linear,0.25;diffusealpha,0);
	};
	
	-- timer background
	LoadActor(THEME:GetPathB("","_frame 3x1"), "headers", 48)..{
		--InitCommand=cmd( x,SCREEN_CENTER_X+(SCREEN_WIDTH/5.35) );
		InitCommand=cmd( x,SCREEN_RIGHT-(SCREEN_WIDTH/3.1825) );
	};
};