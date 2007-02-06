local t = Def.ActorFrame {
	children = {
		NOTESKIN:LoadActor( Var "Button", "Hold Explosion" ) .. {
			HoldingOnCommand=NOTESKIN:GetMetricA("HoldGhostArrow", "HoldingOnCommand");
			HoldingOffCommand=NOTESKIN:GetMetricA("HoldGhostArrow", "HoldingOffCommand");
			InitCommand=cmd(playcommand,"HoldingOff";finishtweening);
		};
		NOTESKIN:LoadActor( Var "Button", "Tap Explosion Dim" ) .. {
			InitCommand=cmd(diffusealpha,0);
			W5Command=NOTESKIN:GetMetricA("GhostArrowDim", "W5Command");
			W4Command=NOTESKIN:GetMetricA("GhostArrowDim", "W4Command");
			W3Command=NOTESKIN:GetMetricA("GhostArrowDim", "W3Command");
			W2Command=NOTESKIN:GetMetricA("GhostArrowDim", "W2Command");
			W1Command=NOTESKIN:GetMetricA("GhostArrowDim", "W1Command");
			HeldCommand=NOTESKIN:GetMetricA("GhostArrowDim", "HeldCommand");
			JudgmentCommand=cmd(finishtweening);
			BrightCommand=cmd(hidden,1);
			DimCommand=cmd(hidden,0);
		};
		NOTESKIN:LoadActor( Var "Button", "Tap Explosion Bright" ) .. {
			InitCommand=cmd(diffusealpha,0);
			W5Command=NOTESKIN:GetMetricA("GhostArrowBright", "W5Command");
			W4Command=NOTESKIN:GetMetricA("GhostArrowBright", "W4Command");
			W3Command=NOTESKIN:GetMetricA("GhostArrowBright", "W3Command");
			W2Command=NOTESKIN:GetMetricA("GhostArrowBright", "W2Command");
			W1Command=NOTESKIN:GetMetricA("GhostArrowBright", "W1Command");
			HeldCommand=NOTESKIN:GetMetricA("GhostArrowBright", "HeldCommand");
			JudgmentCommand=cmd(finishtweening);
			BrightCommand=cmd(hidden,0);
			DimCommand=cmd(hidden,1);
		};
		NOTESKIN:LoadActor( Var "Button", "HitMine Explosion" ) .. {
			InitCommand=cmd(blend,"BlendMode_Add";diffusealpha,0);
			HitMineCommand=NOTESKIN:GetMetricA("GhostArrowBright", "HitMineCommand");
		};
	}
}
return t;
