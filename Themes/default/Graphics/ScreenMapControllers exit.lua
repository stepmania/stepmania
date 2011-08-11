return LoadFont("Common Normal") .. {
	Text="Exit";
	InitCommand=cmd(x,SCREEN_CENTER_X;zoom,0.75;shadowlength,0;diffuse,color("#880000");NoStroke);
	OnCommand=cmd(diffusealpha,0;decelerate,0.5;diffusealpha,1);
	OffCommand=cmd(stoptweening;accelerate,0.3;diffusealpha,0;queuecommand,"Hide");
	HideCommand=cmd(visible,false);

	GainFocusCommand=cmd(diffuseshift;effectcolor1,color("#FF2222");effectcolor2,color("#880000"););
	LoseFocusCommand=cmd(stopeffect);
};
