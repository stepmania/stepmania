return Def.ActorFrame {
	--note graphic
	NOTESKIN:LoadActor(Var "Button", "Tap Note") .. {
		InitCommand=cmd(blend,"BlendMode_Add";playcommand,"Glow");
		W1Command=cmd(playcommand,"Glow");
		W2Command=cmd(playcommand,"Glow");
		W3Command=cmd(playcommand,"Glow");
		W4Command=cmd();
		W5Command=cmd();
		
		HitMineCommand=cmd(playcommand,"Glow");
		GlowCommand=cmd(setstate,0;finishtweening;diffusealpha,1.0;zoom,1.0;linear,0.15;diffusealpha,0.9;zoom,1.15;linear,0.15;diffusealpha,0.0;zoom,1.3);
		HeldCommand=cmd(playcommand,"Glow");
	};
	--tap
	NOTESKIN:LoadActor(Var "Button", "Ready Receptor")..{
		Name="Tap";
		--Frames = { { Frame = 2 ; Delay = 1 } };
		TapCommand=cmd(finishtweening;diffusealpha,1;zoom,1;linear,0.2;diffusealpha,0;zoom,1.2);
		InitCommand=cmd(pause;setstate,2;playcommand,"Tap");
		HeldCommand=cmd(playcommand,"Tap");
		ColumnJudgmentMessageCommand=cmd(playcommand,"Tap");
		--TapNoneCommand=cmd(playcommand,"Tap");
	};
	--explosion
	LoadActor("_flash")..{
		InitCommand=cmd(blend,"BlendMode_Add";playcommand,"Glow");
		W1Command=cmd(playcommand,"Glow");
		W2Command=cmd(playcommand,"Glow");
		W3Command=cmd(playcommand,"Glow");
		W4Command=cmd();
		W5Command=cmd();
		--HoldingOnCommand=cmd(playcommand,"Glow");
		HitMineCommand=cmd(playcommand,"Glow");
		HeldCommand=cmd(playcommand,"Glow");
		GlowCommand=cmd(setstate,0;finishtweening;diffusealpha,1;zoom,1;linear,0.2;diffusealpha,0;zoom,1.2);
	};
	--thing...
	Def.Quad {
		InitCommand=cmd(zoomto,50,5000;diffusealpha,0);
		HitMineCommand=cmd(finishtweening;diffusealpha,1;linear,0.3;diffusealpha,0);
	};
}