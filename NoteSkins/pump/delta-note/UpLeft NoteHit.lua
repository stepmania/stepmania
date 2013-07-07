return NOTESKIN:LoadActor("DownLeft","NoteHit")..{
	InitCommand=function(self)
		self:rotationy(180);
		self:rotationz(180);
		self:y(-6);
		self:x(2);
	end;
}
