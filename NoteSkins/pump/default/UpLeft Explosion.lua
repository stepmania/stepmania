return Def.ActorFrame {
	--note graphic
	NOTESKIN:LoadActor(Var "Button", "Tap Note") .. {
		InitCommand=cmd(blend,"BlendMode_Add";playcommand,"Glow");
		W1Command=cmd(playcommand,"Glow");
		W2Command=cmd(playcommand,"Glow");
		W3Command=cmd(playcommand,"Glow");
		W4Command=cmd();
		W5Command=cmd();
		--HoldingOnCommand=cmd(playcommand,"Glow");
		HitMineCommand=cmd(playcommand,"Glow");
		HeldCommand=cmd(playcommand,"Glow");
		GlowCommand=cmd(setstate,0;finishtweening;diffusealpha,1.0;zoom,1.0;linear,0.15;diffusealpha,0.9;zoom,1.15;linear,0.15;diffusealpha,0.0;zoom,1.3);
	};
	---[[
	NOTESKIN:LoadActor(Var "Button", "Ready Receptor")..{
		Name="Tap";
		Frames = { { Frame = 2 } };
		InitCommand=cmd(zoom,1;diffusealpha,0);
		--NOTESKIN:GetMetricA(Var "Button", "TapInitCommand");
		TapCommand=cmd(finishtweening;diffusealpha,1;zoom,1;linear,0.2;diffusealpha,0;zoom,1.2);
		--NOTESKIN:GetMetricA(Var "Button", "TapHeldCommand");
		
		W1Command=cmd(playcommand,"Tap");
		W2Command=cmd(playcommand,"Tap");
		W3Command=cmd(playcommand,"Tap");
		W4Command=cmd(playcommand,"Tap");
		W5Command=cmd(playcommand,"Tap");
		
		HitMineCommand=cmd(playcommand,"Tap");
		HeldCommand=cmd(playcommand,"Tap");
		
		--NONECommand=cmd(playcommand,"Tap");
		
		--HeldCommand=NOTESKIN:GetMetricA(Var "Button", "TapHeldCommand");
		--NoneCommand=cmd(linear,0.1;zoom,2);
	};
	--]]
	--explosion
	LoadActor("_stepfx")..{
		Frames = {
			{ Frame = 0 ; Delay = 0.05 };
			{ Frame = 1 ; Delay = 0.05 };
			{ Frame = 2 ; Delay = 0.05 };
			{ Frame = 3 ; Delay = 0.05 };
			{ Frame = 4 ; Delay = 0.05 };
			--WUT...
			{ Frame = 5 ; Delay = 99999 };
		};
		InitCommand=cmd(blend,"BlendMode_Add";setstate,0);
		W1Command=cmd(setstate,0);
		W2Command=cmd(setstate,0);
		W3Command=cmd(setstate,0);
		W4Command=cmd();
		W5Command=cmd();
		--HoldingOnCommand=cmd(setstate,0);
		HitMineCommand=cmd(setstate,0);
		HeldCommand=cmd(setstate,1);
	};
	Def.Quad {
		InitCommand=cmd(diffuse,1,1,1,0;zoomto,SCREEN_WIDTH*100,SCREEN_HEIGHT*100;zoomz,SCREEN_WIDTH*SCREEN_HEIGHT);
		HitMineCommand=cmd(finishtweening;diffusealpha,1;linear,0.3;diffusealpha,0);
	};
}