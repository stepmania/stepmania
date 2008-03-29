local stages = Def.ActorFrame {};

function MakeBitmapTest()
	return LoadFont(Var "LoadingScreen","StageDisplay") .. {
		InitCommand=cmd(shadowlength,0;uppercase,true;);
	};
end

for _, s in ipairs(Stage) do
	if s == "Stage_Next" then
		stages[#stages+1] = MakeBitmapTest() .. {
			SetCommand=function(self, params)
				self:visible( params.StageToShow == s );
				self:settext( string.upper(FormatNumberAndSuffix(params.StageIndex+1)) );
				self:diffuse( StageToColor(s) );
				self:strokecolor( StageToStrokeColor(s) );
			end;
		};
	else
		stages[#stages+1] = MakeBitmapTest() .. {
			SetCommand=function(self, params)
				self:visible( params.StageToShow == s );
				self:settext( StageToLocalizedString(params.StageToShow) );
				self:diffuse( StageToColor(s) );
				self:strokecolor( StageToStrokeColor(s) );
			end;
		}
	end
end

return stages;
