local num_players = GAMESTATE:GetHumanPlayers();

local t = LoadFallbackB();

for i=1,#num_players do
	local metrics_name = "PlayerNameplate" .. ToEnumShortString(num_players[i])
	t[#t+1] = LoadActor( THEME:GetPathG(Var "LoadingScreen", "PlayerNameplate"), num_players[i] ) .. {
		InitCommand=function(self)
			self:name(metrics_name);
			ActorUtil.LoadAllCommandsAndSetXY(self,Var "LoadingScreen"); 
		end
	}
end

return t;