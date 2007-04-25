local stages = Def.ActorFrame {
};

for _, s in ipairs(Stage) do
--	if GAMESTATE:IsStagePossible(s) then
	if s == "Stage_Normal" then
		stages[#stages+1] = LoadFont("_shared2") .. {
			InitCommand=cmd(shadowlength,0);
			SetCommand=function(self, params)
				self:visible( params.StageToShow == s );
				if params.StageNumber then
					self:settext( string.format("%i", params.StageNumber+1) );
				end
			end;
		}
	else
		stages[#stages+1] = LoadActor("_stage " .. s) .. {
			SetCommand=function(self, params)
				self:visible( params.StageToShow == s );
			end;
		}
	end
end

return stages;
