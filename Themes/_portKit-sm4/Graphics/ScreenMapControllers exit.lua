return LoadFont("Common Normal") .. {
	Text="Exit";
	InitCommand=cmd(x,SCREEN_CENTER_X;zoom,0.75;shadowlength,0;diffuse,color("#808080"));
	OnCommand=cmd(diffusealpha,0;decelerate,0.5;diffusealpha,1);
	OffCommand=cmd(stoptweening;accelerate,0.3;diffusealpha,0;queuecommand,"Hide");
	HideCommand=cmd(visible,false);

	GainFocusCommand=cmd(diffuseshift;effectcolor2,color("#808080");effectcolor1,color("#FFFFFF"));
	LoseFocusCommand=cmd(stopeffect);
};
