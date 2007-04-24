local stages = Def.ActorFrame {
};

for _, s in ipairs(Stage) do
--	if GAMESTATE:IsStagePossible(s) then
	stages[#stages+1] = LoadActor("_stage " .. s) .. {
		SetCommand=function(self, params) self:visible( params.StageToShow == s ) end;
	}
end

return stages;
