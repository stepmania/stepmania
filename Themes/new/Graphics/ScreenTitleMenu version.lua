return LoadFont("Common Normal") .. {
	Name="Version";
	Text=string.format(THEME:GetString( Var "LoadingScreen", "Version" ), ProductVersion() );
};