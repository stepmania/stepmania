local stages = Def.ActorFrame {
	BeginCommand=cmd(playcommand,"Set";);
	CurrentSongChangedMessageCommand=cmd(playcommand,"Set";);
};

local ScreenName = Var "LoadingScreen";

function MakeBitmapTest()
	return LoadFont(ScreenName,"StageDisplay") .. {
		InitCommand=cmd(shadowlength,0;uppercase,true;);
	};
end

for _, s in ipairs(Stage) do
	if s == "Stage_Next" then
		stages[#stages+1] = MakeBitmapTest() .. {
			SetCommand=function(self, params)
				local StageToShow = THEME:GetMetric( ScreenName, "StageDisplayStageToShow" );
				local StageIndex = THEME:GetMetric( ScreenName, "StageDisplayStageIndex" );
				self:visible( StageToShow == s );
				self:settext( string.upper(FormatNumberAndSuffix(StageIndex+1)) );
				self:diffuse( StageToColor(s) );
				self:strokecolor( StageToStrokeColor(s) );
			end;
		};
	else
		stages[#stages+1] = MakeBitmapTest() .. {
			SetCommand=function(self, params)
				local StageToShow = THEME:GetMetric( ScreenName, "StageDisplayStageToShow" );
				self:visible( StageToShow == s );
				self:settext( StageToLocalizedString(StageToShow) );
				self:diffuse( StageToColor(s) );
				self:strokecolor( StageToStrokeColor(s) );
			end;
		}
	end
end

return stages;
