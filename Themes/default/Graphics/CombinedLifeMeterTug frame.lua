return LoadActor(THEME:GetPathG("_combined","life frame"))..{
	InitCommand=function(self)
		self:diffuse(PlayerColor(PLAYER_1));
		self:diffuserightedge(PlayerColor(PLAYER_2));
	end;
};