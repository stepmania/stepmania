local stages = Def.ActorFrame {
	LoadActor("_stage frame");
};

for _, s in ipairs(Stage) do
	if s == "Stage_Normal" then
		stages[#stages+1] = LoadFont("_sf square head bold stroke 28") .. {
			InitCommand=cmd(shadowlength,0;zoom,0.5;diffuse,color("#208f00"););
			SetCommand=function(self, params)
				self:visible( params.StageToShow == s );
				if params.StageNumber then
					self:settext( string.upper(FormatNumberAndSuffix(params.StageNumber+1)) );
				end
			end;
		};
		stages[#stages+1] = LoadFont("_sf square head bold 28") .. {
			InitCommand=cmd(shadowlength,0;zoom,0.5;diffuse,color("#6cff00"););
			SetCommand=function(self, params)
				self:visible( params.StageToShow == s );
				if params.StageNumber then
					self:settext( string.upper(FormatNumberAndSuffix(params.StageNumber+1)) );
				end
			end;
		};
	else
		stages[#stages+1] = LoadActor("_stage " .. s) .. {
			SetCommand=function(self, params)
				self:visible( params.StageToShow == s );
			end;
		}
	end
end

return stages;
