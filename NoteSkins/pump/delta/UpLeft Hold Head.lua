
local t = Def.ActorFrame{}


t[#t+1] = LoadActor("UpLeft_blend")..{
	InitCommand=cmd(diffuseshift;effectcolor1,color("#9b376dFF");effectcolor2,color("#9b376dFF");fadetop,0.5);
}

t[#t+1] = LoadActor("UpLeft_blend")..{
	InitCommand=cmd(blend,Blend.Add;diffuseshift;effectcolor1,color("#cc5176FF");effectcolor2,color("#cc517633");effectclock,"bgm";effecttiming,1,0,0,0;);
}

t[#t+1] = LoadActor("UpLeft_fill")..{
	InitCommand=cmd(blend,Blend.Add;diffuseshift;effectcolor1,color("#cc5176FF");effectcolor2,color("#cc5176FF"));
}


t[#t+1] = LoadActor("UpLeft border");


return t