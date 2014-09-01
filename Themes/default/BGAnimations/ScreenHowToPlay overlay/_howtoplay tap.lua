return Def.ActorFrame {
	LoadActor(THEME:GetPathB("_frame","3x3"),"rounded black",392+8,80);
	Def.Quad {
		Name="Underline";
		InitCommand=cmd(y,-10);
		OnCommand=cmd(diffuse,color("#ffd400");shadowlength,1;zoomtowidth,192;fadeleft,0.25;faderight,0.25);
	};
	LoadFont("Common Bold") .. {
		Text="Information";
		InitCommand=cmd(y,-24);
		OnCommand=cmd(skewx,-0.125;diffuse,color("#ffd400");shadowlength,2;shadowcolor,BoostColor(color("#ffd40077"),0.25))
	};
	LoadFont("Common Normal") .. {
		Text="When the arrows rise to this point,\nstep on the matching panels.";
		InitCommand=cmd(y,18;wrapwidthpixels,480;vertspacing,-12;shadowlength,1);
		OnCommand=cmd(zoom,0.875);
	};
};
