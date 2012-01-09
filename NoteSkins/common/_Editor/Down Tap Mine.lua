local t = Def.ActorFrame {
	Def.Sprite {
		Texture=NOTESKIN:GetPath( '_down', 'tap mine underlay' );
		Frames = Sprite.LinearFrames( 1, 1 );
		InitCommand=cmd(diffuseshift;effectcolor1,0.4,0,0,1;effectcolor2,1,0,0,1;effectclock,'beat');
	};
		Def.Sprite {
		Texture=NOTESKIN:GetPath( '_down', 'tap mine base' );
		Frames = Sprite.LinearFrames( 1, 1 );
		InitCommand=cmd(spin;effectclock,'beat';effectmagnitude,0,0,80);
	};
		Def.Sprite {
		Texture=NOTESKIN:GetPath( '_down', 'tap mine overlay' );
		Frames = Sprite.LinearFrames( 1, 1 );
		InitCommand=cmd(spin;effectclock,'beat';effectmagnitude,0,0,-40);
	};
};
return t;
