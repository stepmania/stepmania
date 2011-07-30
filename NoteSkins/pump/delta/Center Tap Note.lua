local t = Def.ActorFrame{}


t[#t+1] = LoadActor("Center_blend")..{
	InitCommand=cmd(diffuseshift;effectcolor1,color("#9b873766");effectcolor2,color("#9b873766"));
}

t[#t+1] = LoadActor("Center_blend")..{
	InitCommand=cmd(blend,Blend.Add;diffuseshift;effectcolor1,color("#ccb752FF");effectcolor2,color("#ccb75266");effectclock,"bgm";effecttiming,1,0,0,0;);
}

t[#t+1] = LoadActor("Center_blend")..{
	InitCommand=cmd(diffuseshift;effectcolor1,color("#ccb752FF");effectcolor2,color("#ccb752FF");fadetop,1);
}

t[#t+1] = LoadActor("Center_fill")..{
	InitCommand=cmd(blend,Blend.Add;diffuseshift;effectcolor1,color("#ccb752FF");effectcolor2,color("#ccb752FF"));
}

t[#t+1] = LoadActor("Center_feet")..{
	InitCommand=cmd(blend,Blend.Add;diffusealpha,0.6);
}


t[#t+1] = LoadActor("Center border");


return t