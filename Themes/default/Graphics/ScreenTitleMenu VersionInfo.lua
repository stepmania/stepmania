return Def.ActorFrame {
	LoadActor(THEME:GetPathG("ScreenTitleMenu","PreferenceFrame")) .. {
		OnCommand=cmd(diffuse,Color("Orange");diffusetopedge,Color("Yellow");diffusealpha,0.25);
	};
	LoadFont("Common Normal") .. {
		Text=ProductID();
		AltText="";
		InitCommand=cmd(x,-72;y,-1;horizalign,left;zoom,0.75);
		OnCommand=cmd(shadowlength,1);
	};
	LoadFont("Common Normal") .. {
		Text=ProductVersion() .. "\n" .. VersionDate(); 
		AltText="";
		InitCommand=cmd(x,72*0.35;zoom,0.5);
		OnCommand=cmd(shadowlength,1;skewx,-0.125);
	};
--[[ 	LoadFont("Common Normal") .. {
		OnCommand=cmd(settext,"You're using " ..ProductID().." "..ProductVersion().."\nBuilt on "..VersionDate().." at "..VersionTime();horizalign,right;vertalign,bottom;zoom,0.5;);
	}; --]]
};