local File = ...;

return LoadActor( File ) .. {
	InitCommand=function(self)
		self:playcommand("Lift");
	end;
	ReverseOnCommand=NOTESKIN:GetMetricA("Press", "ReverseOnCommand");
	ReverseOffCommand=NOTESKIN:GetMetricA("Press", "ReverseOffCommand");
	PressCommand=NOTESKIN:GetMetricA("Press", "PressCommand");
	LiftCommand=NOTESKIN:GetMetricA("Press", "LiftCommand");
};
