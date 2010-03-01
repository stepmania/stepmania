return Def.ActorFrame {
	NOTESKIN:LoadActor(Var "Button", "Tap Note") .. {
		InitCommand=cmd(blend,"BlendMode_Add";playcommand,"Glow");
		W1Command=cmd(playcommand,"Glow");
		W2Command=cmd(playcommand,"Glow");
		W3Command=cmd(playcommand,"Glow");
		W4Command=cmd();
		W5Command=cmd();
		HoldingOnCommand=cmd(playcommand,"Glow");
		HitMineCommand=cmd(playcommand,"Glow");
		HeldCommand=cmd(playcommand,"Glow");
		GlowCommand=cmd(setstate,0;finishtweening;diffusealpha,1.0;zoom,1.0;linear,0.15;diffusealpha,0.9;zoom,1.1;linear,0.15;diffusealpha,0.0;zoom,1.2);
	};
	Def.Quad {
		InitCommand=cmd(diffuse,1,1,1,0;zoomto,SCREEN_WIDTH*100,SCREEN_HEIGHT*100;zoomz,SCREEN_WIDTH*SCREEN_HEIGHT);
		HitMineCommand=cmd(finishtweening;diffusealpha,1;linear,0.3;diffusealpha,0);
	};
}