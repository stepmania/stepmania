local t = Def.ActorFrame {
	NOTESKIN:LoadActor( Var "Button", "Hold Explosion" ) .. {
		HoldingOnCommand=NOTESKIN:GetMetricA("HoldGhostArrow", "HoldingOnCommand");
		HoldingOffCommand=NOTESKIN:GetMetricA("HoldGhostArrow", "HoldingOffCommand");
		InitCommand=function(self)
			self:playcommand("HoldingOff");
			self:finishtweening();
		end;
	};
	NOTESKIN:LoadActor( Var "Button", "Roll Explosion" ) .. {
		RollOnCommand=NOTESKIN:GetMetricA("HoldGhostArrow", "RollOnCommand");
		RollOffCommand=NOTESKIN:GetMetricA("HoldGhostArrow", "RollOffCommand");
		InitCommand=function(self)
			self:playcommand("RollOff");
			self:finishtweening();
		end;
	};
	NOTESKIN:LoadActor( Var "Button", "Tap Explosion Dim" ) .. {
		InitCommand=function(self)
			self:diffusealpha(0);
		end;
		W5Command=NOTESKIN:GetMetricA("GhostArrowDim", "W5Command");
		W4Command=NOTESKIN:GetMetricA("GhostArrowDim", "W4Command");
		W3Command=NOTESKIN:GetMetricA("GhostArrowDim", "W3Command");
		W2Command=NOTESKIN:GetMetricA("GhostArrowDim", "W2Command");
		W1Command=NOTESKIN:GetMetricA("GhostArrowDim", "W1Command");
		HeldCommand=NOTESKIN:GetMetricA("GhostArrowDim", "HeldCommand");
		JudgmentCommand=function(self)
			self:finishtweening();
		end;
		BrightCommand=function(self)
			self:visible(false);
		end;
		DimCommand=function(self)
			self:visible(true);
		end;
	};
	NOTESKIN:LoadActor( Var "Button", "Tap Explosion Bright" ) .. {
		InitCommand=function(self)
			self:diffusealpha(0);
		end;
		W5Command=NOTESKIN:GetMetricA("GhostArrowBright", "W5Command");
		W4Command=NOTESKIN:GetMetricA("GhostArrowBright", "W4Command");
		W3Command=NOTESKIN:GetMetricA("GhostArrowBright", "W3Command");
		W2Command=NOTESKIN:GetMetricA("GhostArrowBright", "W2Command");
		W1Command=NOTESKIN:GetMetricA("GhostArrowBright", "W1Command");
		HeldCommand=NOTESKIN:GetMetricA("GhostArrowBright", "HeldCommand");
		JudgmentCommand=function(self)
			self:finishtweening();
		end;
		BrightCommand=function(self)
			self:visible(true);
		end;
		DimCommand=function(self)
			self:visible(false);
		end;
	};
	NOTESKIN:LoadActor( Var "Button", "HitMine Explosion" ) .. {
		InitCommand=function(self)
			self:blend("BlendMode_Add");
			self:diffusealpha(0);
		end;
		HitMineCommand=NOTESKIN:GetMetricA("GhostArrowBright", "HitMineCommand");
	};
}
return t;
