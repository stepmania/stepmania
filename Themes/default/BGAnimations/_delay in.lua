local fSeconds = ScreenMetric("InDelay");
return Def.Actor { OnCommand=cmd(sleep,fSeconds); };

