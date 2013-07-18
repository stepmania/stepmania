local t = Def.ActorFrame{}


t[#t+1] = LoadActor("DownLeft_blend")..{
	InitCommand=cmd(diffuseshift;effectcolor1,color("#376f9b66");effectcolor2,color("#376f9b66"));
}

t[#t+1] = LoadActor("DownLeft_blend")..{
	InitCommand=cmd(blend,Blend.Add;diffuseshift;effectcolor1,color("#5899ccFF");effectcolor2,color("#5899cc66");effectclock,"bgm";effecttiming,1,0,0,0;);
}

t[#t+1] = LoadActor("DownLeft_blend")..{
	InitCommand=cmd(diffuseshift;effectcolor1,color("#5899ccFF");effectcolor2,color("#5899ccFF");fadetop,1);
}

t[#t+1] = LoadActor("DownLeft_fill")..{
	InitCommand=cmd(blend,Blend.Add;diffuseshift;effectcolor1,color("#5899ccFF");effectcolor2,color("#5899ccFF"));
}


t[#t+1] = LoadActor("DownLeft border");


return t