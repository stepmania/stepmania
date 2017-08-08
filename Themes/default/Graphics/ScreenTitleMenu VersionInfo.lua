return Def.ActorFrame {
	LoadFont("Common Condensed") .. {
		Text=string.format("%s %s", ProductFamily(), ProductVersion());
		AltText="StepMania";
		InitCommand=cmd(zoom,1);
		OnCommand=cmd(horizalign,right;diffusealpha,0.9);
	};
	LoadFont("Common Normal") .. {
		Text=string.format("%s", VersionDate());
		AltText="Unknown Version";
		InitCommand=cmd(y,19;zoom,0.75);
		OnCommand=cmd(horizalign,right;diffusealpha,0.7);
	};
};