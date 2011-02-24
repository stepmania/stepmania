return NOTESKIN:LoadActor("DownRight","Tap Note")..{
	InitCommand=cmd(effectclock,"beat";effectmagnitude,0.5,1,0)
};