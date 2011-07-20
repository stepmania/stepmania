local stages = Def.ActorFrame {
	BeginCommand=cmd(playcommand,"Set";);
	CurrentSongChangedMessageCommand=cmd(finishtweening;playcommand,"Set";);
};

local ScreenName = Var "LoadingScreen";

function MakeBitmapTest()
	return LoadFont(ScreenName,"StageDisplay") .. {

	};
end

for s in ivalues(Stage) do
	stages[#stages+1] = MakeBitmapTest() .. {
		InitCommand=cmd(shadowlength,2);
		SetCommand=function(self, params)
			local Stage = GAMESTATE:GetCurrentStage();
			local StageIndex = GAMESTATE:GetCurrentStageIndex();
			local screen = SCREENMAN:GetTopScreen();
			local cStageOutlineColor = ColorDarkTone( StageToStrokeColor(s) );
			cStageOutlineColor[4] = 0.75;
			if screen and screen.GetStageStats then
				local ss = screen:GetStageStats();
				Stage = ss:GetStage();
				StageIndex = ss:GetStageIndex();
			end
			self:visible( Stage == s );
			self:settext( StageToLocalizedString(Stage) );
			self:diffuse( StageToColor(s) );
			self:diffusebottomedge( ColorMidTone(StageToColor(s)) );
			self:strokecolor( cStageOutlineColor );
		end;
	}
end

return stages;
