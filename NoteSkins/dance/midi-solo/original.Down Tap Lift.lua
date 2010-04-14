local t = Def.ActorFrame {
	Def.Sprite {
		Texture=NOTESKIN:GetPath( '_down', 'tap lift' );
		InitCommand=cmd(diffusealpha,0.5);
	};
	Def.Sprite {
		Texture=NOTESKIN:GetPath( '_down', 'tap lift' );
		InitCommand=cmd(pulse;effectclock,"beat";effecttiming,0,0,0.75/2,0.25/2;effectmagnitude,1,1,1;effectcolor1,color("1,1,1,1");effectcolor2,color("1,1.25,1,1"));
	};
	Def.Sprite {
		Texture=NOTESKIN:GetPath( '_down', 'lift fill' );
		Frames = Sprite.LinearFrames( 4, 1 );
		InitCommand=cmd(pulse;effectclock,"beat";effecttiming,0,0,0.75/4,0.25/4;effectmagnitude,1,1,1;effectcolor1,color("1,1,1,1");effectcolor2,color("1,1,1,1"));
	};
};
return t;
