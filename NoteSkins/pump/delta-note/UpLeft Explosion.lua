return Def.ActorFrame {

	NOTESKIN:LoadActor(Var "Button","NoteHit") .. {
		InitCommand=cmd(animate,0;blend,Blend.Add;diffusealpha,0);

		NoneCommand=cmd(playcommand,"Glow");
		PressCommand=cmd(playcommand,"Glow");
		W1Command=cmd(setstate,0;playcommand,"W2");
		W2Command=cmd(setstate,0;playcommand,"Glow");
		W3Command=cmd(setstate,1;playcommand,"Glow");
		W4Command=cmd(setstate,2;playcommand,"Glow");
		W5Command=cmd();
		HitMineCommand=cmd(playcommand,"Glow");
		HeldCommand=cmd(setstate,0;playcommand,"Glow");
		GlowCommand=cmd(stoptweening,zoom,1.05;diffusealpha,1;linear,0.25;zoom,1.1;diffusealpha,0);
	};

	NOTESKIN:LoadActor(Var "Button","NoteHit") .. {
		InitCommand=cmd(animate,0;blend,Blend.Add;diffusealpha,0);

		NoneCommand=cmd(playcommand,"Glow");
		PressCommand=cmd(playcommand,"Glow");
		W1Command=cmd(setstate,0;playcommand,"W2");
		W2Command=cmd(setstate,0;playcommand,"Glow");
		W3Command=cmd(setstate,1;playcommand,"Glow");
		W4Command=cmd(setstate,2;playcommand,"Glow");
		W5Command=cmd();
		HitMineCommand=cmd(playcommand,"Glow");
		HeldCommand=cmd(setstate,0;playcommand,"Glow");
		GlowCommand=cmd(stoptweening,zoom,1;diffusealpha,0.4;linear,0.3;zoom,1.2;diffusealpha,0);
	};

	NOTESKIN:LoadActor(Var "Button","NoteHit") .. {
		InitCommand=cmd(animate,0;zoom,1.1;blend,Blend.Add;visible,false);
		HoldingOnCommand=cmd(visible,true);
		HoldingOffCommand=cmd(visible,false);
	};


}