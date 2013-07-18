local t = Def.ActorFrame{}


t[#t+1] = LoadActor("DownLeft_blend")..{
	InitCommand=cmd(diffuseshift;effectcolor1,color("#376f9bFF");effectcolor2,color("#376f9bFF");fadetop,0.5);
}

t[#t+1] = LoadActor("DownLeft_blend")..{
	InitCommand=cmd(blend,Blend.Add;diffuseshift;effectcolor1,color("#5899ccFF");effectcolor2,color("#5899cc33");effectclock,"bgm";effecttiming,1,0,0,0;);
}

t[#t+1] = LoadActor("DownLeft_fill")..{
	InitCommand=cmd(blend,Blend.Add;diffuseshift;effectcolor1,color("#5899ccFF");effectcolor2,color("#5899ccFF"));
}


t[#t+1] = LoadActor("DownLeft border");


return t