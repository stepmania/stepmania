return LoadActor(THEME:GetPathG("_combined","life frame"))..{
	InitCommand=cmd(diffuse,PlayerColor(PLAYER_1);diffuserightedge,PlayerColor(PLAYER_2));
};