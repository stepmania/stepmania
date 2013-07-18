return Def.ActorFrame {
	-- We want this under the noteskin, so that we it looks like a laser (?)
	LoadActor( NOTESKIN:GetPath("", "_Tap Receptor"), NOTESKIN:LoadActor( Var "Button", "KeypressBlock" ) ) .. {
		InitCommand=cmd(vertalign,top;zoomx,0);
		-- Press/Lift allows this to appear and disappear
		PressCommand=cmd(zoomx,0;linear,0.02;zoomx,1);
		LiftCommand=cmd(zoomx,1;linear,0.14;zoomx,0);
	};
	-- Overlay the receptor.
	LoadActor( NOTESKIN:GetPath("", "_Tap Receptor"), NOTESKIN:LoadActor( Var "Button", "Go Receptor" ) );	
};

