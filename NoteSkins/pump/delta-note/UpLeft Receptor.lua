return Def.ActorFrame {
	NOTESKIN:LoadActor(Var "Button", "Ready Receptor")..{
		Name="Base";
		InitCommand=cmd(animate,0;setstate,0);
	};

	NOTESKIN:LoadActor(Var "Button", "Ready Receptor")..{
		Name="Glow";
		InitCommand=cmd(animate,0;setstate,1);
		OnCommand=cmd(effectclock,"bgm";diffuseshift;effectcolor1,color("#FFFFFFFF");effectcolor2,color("#FFFFFF00");effecttiming,1,0,0,0);
	};

	NOTESKIN:LoadActor(Var "Button", "Ready Receptor")..{
		Name="Tap";
		InitCommand=cmd(animate,0;setstate,2;zoom,1;diffusealpha,0;blend,'BlendMode_Add');
		PressCommand=cmd(diffuse,color("#FFFFAA");stoptweening;zoom,1.1;linear,0.1;diffusealpha,0.6;zoom,1);
		LiftCommand=cmd(diffuse,color("#FFFFAA");stoptweening;diffusealpha,0.6;zoom,1;linear,0.15;zoom,1.2;diffusealpha,0);
	};
	
}

--Vin.il was here... =)