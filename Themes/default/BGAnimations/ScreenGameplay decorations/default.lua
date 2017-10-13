local t = Def.ActorFrame {};

-- LifeMeterBar
for i, pn in pairs(PlayerNumber) do
	if ShowStandardDecoration("LifeMeterBar" ..  ToEnumShortString(pn)) then
		local life_type = GAMESTATE:GetPlayerState(pn):get_player_options_no_defect("ModsLevel_Song"):LifeSetting()
		t[#t+1] = LoadActor(THEME:GetPathG(Var "LoadingScreen", "lifebar_" .. ToEnumShortString(life_type)), pn) .. {
			InitCommand=function(self)
				self:name("LifeMeterBar" .. ToEnumShortString(pn))
				ActorUtil.LoadAllCommandsAndSetXY(self,Var "LoadingScreen")
			end
		}
	end
end

return t