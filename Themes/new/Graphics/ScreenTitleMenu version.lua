return LoadFont("Common Normal") .. {
	Text=string.format("%s [%s]", ProductVersion(), VersionDate());
	AltText="StepMania";
};