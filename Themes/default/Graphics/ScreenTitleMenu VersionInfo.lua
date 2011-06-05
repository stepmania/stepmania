return Def.ActorFrame {
	LoadActor(THEME:GetPathG("ScreenTitleMenu","PreferenceFrame")) .. {
		OnCommand=cmd(diffuse,Color("Orange");diffusetopedge,Color("Yellow");diffusealpha,0.25);
	};
	LoadFont("Common Normal") .. {
		Text=ProductID();
		AltText="";
		InitCommand=cmd(y,-3;zoom,0.75);
		OnCommand=cmd(shadowlength,1);
	};
	LoadFont("Common Normal") .. {
		Text=ProductVersion() .. " (".. VersionDate() ..")";
		AltText="";
		InitCommand=cmd(y,13;zoom,0.5);
		OnCommand=cmd(shadowlength,1;skewx,-0.125);
	};
};