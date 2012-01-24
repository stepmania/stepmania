return Def.Sprite {
	Texture=NOTESKIN:GetPath( '_center', 'explosion' );
	W1Command=cmd(blend,"BlendMode_Add";finishtweening;diffusealpha,0.2;zoom,0.6;linear,0.1;diffusealpha,0;zoom,0.8);
	W2Command=cmd(blend,"BlendMode_Add";finishtweening;diffusealpha,0.2;zoom,0.6;linear,0.1;diffusealpha,0;zoom,0.8);
	W3Command=cmd(blend,"BlendMode_Add";finishtweening;diffusealpha,0.2;zoom,0.6;linear,0.1;diffusealpha,0;zoom,0.8);
	W4Command=cmd(blend,"BlendMode_Add";finishtweening;diffusealpha,0.2;zoom,0.6;linear,0.1;diffusealpha,0;zoom,0.8);
	W5Command=cmd(blend,"BlendMode_Add";finishtweening;diffusealpha,0.2;zoom,0.6;linear,0.1;diffusealpha,0;zoom,0.8);
};