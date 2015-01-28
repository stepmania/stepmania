local t = Def.ActorFrame {
	InitCommand=cmd(pulse;effectclock,'beat';effectmagnitude,1,1.2,1);
	Def.Sprite {
		Texture=NOTESKIN:GetPath( '_AnyRightFist', 'tap lift' );
		Frame0000=0;
		Delay0000=1;
		InitCommand=cmd(diffuseramp;effectclock,'beat';effectcolor1,color("1,1,1,1");effectcolor2,color("1,1,1,0.5"));
	};
};
return t;
