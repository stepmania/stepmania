return Def.BPMDisplay {
	File=THEME:GetPathF("BPMDisplay", "bpm");
	Name="BPMDisplay";
	SetCommand=function(self) self:SetFromGameState() end;
	CurrentSongChangedMessageCommand=function(self)
		self:playcommand("Set");
	end;
	CurrentCourseChangedMessageCommand=function(self)
		self:playcommand("Set");
	end;
};