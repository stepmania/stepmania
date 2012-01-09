return Def.Sprite {
	Texture=NOTESKIN:GetPath( '_down', 'explosion' );
	Frame0000=0;
	Delay0000=1;
	InitCommand=cmd(diffuseblink;effectcolor1,1,1,1,0.8;effectcolor2,1,1,1,1;effectclock,'beat';effectperiod,0.25);
};