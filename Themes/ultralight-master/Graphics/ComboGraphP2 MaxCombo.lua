return Def.Quad{
	InitCommand=cmd(setsize,1,12;diffuse,PlayerColor(PLAYER_2);diffusetopedge,Saturation(Brightness(PlayerColor(PLAYER_2),1),0.5););
	BeginCommand=cmd(glowshift;effectcolor1,color("1,1,1,0.325");effectcolor2,color("1,1,1,0"););
};