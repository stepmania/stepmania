return LoadFont("Common normal") .. {
	Text="Exit";
	OnCommand=cmd(diffusealpha,0;linear,0.3;diffusealpha,1);
	OffCommand=cmd(linear,0.3;diffusealpha,0);
	InitCommand=cmd(halign,0;shadowlength,2);
	GainFocusCommand=cmd(stoptweening;diffuseshift;effectperiod,0.5;effectcolor1,1,0.5,0.5,1;effectcolor2,0.5,0.25,0.25,1;linear,0;Zoom,1.0);
	LoseFocusCommand=cmd(stoptweening;stopeffect;linear,0;zoom,0.9);
	EnabledCommand=cmd();
	DisabledCommand=cmd(diffuse,0.5,0.5,0.5,1);
};

