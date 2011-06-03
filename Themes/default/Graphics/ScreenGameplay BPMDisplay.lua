return Def.SongBPMDisplay {
	File=THEME:GetPathF("BPMDisplay", "bpm");
	Name="BPMDisplay";
	InitCommand=cmd(zoom,0.675;shadowlength,1);
	SetCommand=function(self) self:SetFromGameState() end;
	CurrentSongChangedMessageCommand=cmd(playcommand,"Set");
	CurrentCourseChangedMessageCommand=cmd(playcommand,"Set");
};