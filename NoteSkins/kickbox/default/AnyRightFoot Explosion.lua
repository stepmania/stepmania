local t = Def.ActorFrame {
	LoadActor( "_AnyRightFoot Hold Explosion" ) .. {
		HoldingOnCommand=NOTESKIN:GetMetricA("HoldGhostArrow", "HoldingOnCommand");
		HoldingOffCommand=NOTESKIN:GetMetricA("HoldGhostArrow", "HoldingOffCommand");
		InitCommand=cmd(playcommand,"HoldingOff";finishtweening);
	};
	LoadActor( "_AnyRightFoot Roll Explosion" ) .. {
		RollOnCommand=NOTESKIN:GetMetricA("HoldGhostArrow", "RollOnCommand");
		RollOffCommand=NOTESKIN:GetMetricA("HoldGhostArrow", "RollOffCommand");
		InitCommand=cmd(playcommand,"RollOff";finishtweening);
	};
	LoadActor( "_AnyRightFoot Tap Explosion Dim" ) .. {
		InitCommand=cmd(diffusealpha,0);
		W5Command=NOTESKIN:GetMetricA("GhostArrowDim", "W5Command");
		W4Command=NOTESKIN:GetMetricA("GhostArrowDim", "W4Command");
		W3Command=NOTESKIN:GetMetricA("GhostArrowDim", "W3Command");
		W2Command=NOTESKIN:GetMetricA("GhostArrowDim", "W2Command");
		W1Command=NOTESKIN:GetMetricA("GhostArrowDim", "W1Command");
		HeldCommand=NOTESKIN:GetMetricA("GhostArrowDim", "HeldCommand");
		JudgmentCommand=cmd(finishtweening);
		BrightCommand=cmd(visible,false);
		DimCommand=cmd(visible,true);
	};
	LoadActor( "AnyRightFoot HitMine Explosion" ) .. {
		InitCommand=cmd(blend,"BlendMode_Add";diffusealpha,0);
		HitMineCommand=NOTESKIN:GetMetricA("GhostArrowBright", "HitMineCommand");
	};
}
return t;
