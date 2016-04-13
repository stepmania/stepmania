local time = ...
assert(time)
return Def.Quad{
	InitCommand=cmd(FullScreen;diffuse,HSV(192,1,0.05);diffusebottomedge,HSV(192,0.75,0.125);cropbottom,0;fadebottom,0);
	OnCommand=cmd(linear,time;cropbottom,1;fadebottom,1;);
};