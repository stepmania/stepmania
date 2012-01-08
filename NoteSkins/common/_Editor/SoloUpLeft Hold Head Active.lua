local t = Def.ActorFrame {
	Def.Sprite {
		Texture=NOTESKIN:GetPath( '_upleftsolo', 'underlay' );
		Frame0000=0;
		Delay0000=1;
		InitCommand=cmd();
	};
	Def.Sprite {
		Texture=NOTESKIN:GetPath( '_down', 'tap note' );
		Frame0000=0;
		Delay0000=1;
		InitCommand=cmd(rotationz,135);
	};
};
return t;