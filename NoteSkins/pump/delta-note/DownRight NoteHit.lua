return NOTESKIN:LoadActor("DownLeft","NoteHit")..{
	OnCommand=function(self)
		self:x(-2);
		self:zoomx(-1);
	end;
}