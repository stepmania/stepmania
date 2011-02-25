return NOTESKIN:LoadActor("DownLeft","Tap Note")..{
	InitCommand=cmd(effectclock,"beat";effectmagnitude,0.5,1,0)
};