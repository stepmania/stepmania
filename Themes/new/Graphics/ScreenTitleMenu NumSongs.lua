return LoadFont("Common Normal") .. {
	Text=string.format("%i songs in %i groups, %i courses", SONGMAN:GetNumSongs(), SONGMAN:GetNumSongGroups(), SONGMAN:GetNumCourses() );
	AltText="StepMania";
};