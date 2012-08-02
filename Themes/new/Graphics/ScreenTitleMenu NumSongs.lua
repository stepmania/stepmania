return LoadFont("Common Normal") .. {
	Name="NumSongs";
	Text=string.format(THEME:GetString( Var "LoadingScreen", "NumSongs" ), SONGMAN:GetNumSongs(), SONGMAN:GetNumSongGroups(), SONGMAN:GetNumCourses() );
};