local t = Def.ActorFrame{};

t[#t+1] = LoadActor("_badge") .. {
	InitCommand=cmd(shadowlength,1;diffuse,Color.Green;pulse;effectmagnitude,0.75,1,1;effecttiming,0,0,1,0);
};
t[#t+1] = LoadFont("Common","Normal") .. {
	Text="AG";
	InitCommand=cmd(shadowlength,1;zoom,0.875);
};

return t;