return LoadActor( NOTESKIN:GetPath("", "_Tap Receptor"), NOTESKIN:LoadActor( "Left", "Go Receptor" ) )..{
	InitCommand=cmd(zoomx,-1);
};