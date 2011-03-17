local t = ...;
t = Def.Sprite {
	Texture="Hold Explosion.png";
	HoldingOnCommand=NOTESKIN:GetMetricA("HoldGhostArrow", "HoldingOnCommand");
	HoldingOffCommand=NOTESKIN:GetMetricA("HoldGhostArrow", "HoldingOffCommand");
	InitCommand=NOTESKIN:GetMetricA("HoldGhostArrow", "InitCommand");
};

return t;