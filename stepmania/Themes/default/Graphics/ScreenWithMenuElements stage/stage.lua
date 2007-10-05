local stages = Def.ActorFrame {
};

for _, s in ipairs(Stage) do
	if s == "Stage_Normal" then
		stages[#stages+1] = LoadFont("_zeroesthree") .. {
			InitCommand=cmd(shadowlength,0);
			SetCommand=function(self, params)
				self:visible( params.StageToShow == s );
				if params.StageNumber then
					self:settext( FormatNumberAndSuffix(params.StageNumber+1) );
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
