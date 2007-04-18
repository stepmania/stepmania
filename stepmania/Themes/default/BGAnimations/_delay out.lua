local fSeconds = ScreenMetric("OutDelay");
return Def.Actor { OnCommand=cmd(sleep,fSeconds); };
