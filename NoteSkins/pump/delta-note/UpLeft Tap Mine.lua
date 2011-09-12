local t = Def.ActorFrame{}


t[#t+1] = LoadActor("Mine_Base");

t[#t+1] = LoadActor("Mine_Fill")..{
	InitCommand=cmd(diffuseshift;effectcolor1,color("#FFFFFFFF");effectcolor2,color("#FFFFFF22");effectclock,"bgm";effectperiod,2);
}
t[#t+1] = LoadActor("Mine_Fill")..{
	InitCommand=cmd(blend,Blend.Add;diffuseshift;effectcolor1,color("#FFFFFFFF");effectcolor2,color("#FFFFFF22");effectclock,"bgm";effectperiod,2);
}


t[#t+1] = LoadActor("Mine_Border")..{
	InitCommand=cmd(spin;effectmagnitude,0,0,36);
}

t[#t+1] = LoadActor("Mine_Overlay");



t[#t+1] = LoadActor("Mine_Light")..{
	InitCommand=cmd(blend,Blend.Add;diffuseshift;effectcolor1,color("#FFFFFF55");effectcolor2,color("#FFFFFF00");effectclock,"bgm";zoom,1.15;effectperiod,2);
}

return t