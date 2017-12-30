local t = Def.ActorFrame {
	Def.Sprite {
		Texture=NOTESKIN:GetPath( '_down', 'tap lift' );
		InitCommand=cmd(animate,false;pulse;effectclock,"beat";effectmagnitude,0.9,1,1;effectcolor1,color("1,1,1,1");effectcolor2,color("1,1,1,0.8"););
	};
};
return t;
