local t = Def.ActorFrame{};

t[#t+1] = LoadActor("_badge") .. {
	InitCommand=cmd(shadowlength,1;diffuse,Color.Green;pulse;effectmagnitude,0.875,1,1;effecttiming,0,0,1,0
		effectclock,'beatnooffset';effectperiod,2);
};
t[#t+1] = Def.Quad {
	InitCommand=cmd(zoomto,40,20;diffuse,Color.Black;
		diffusealpha,0.5;fadeleft,0.25;faderight,0.25);
};
t[#t+1] = Def.BitmapText {
	Font="Common Normal";
	Text="AG";
	InitCommand=cmd(shadowlength,1;zoom,0.875);
};

return t;