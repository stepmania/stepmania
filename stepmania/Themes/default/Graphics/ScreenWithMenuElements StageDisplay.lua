local stages = Def.ActorFrame {
	BeginCommand=cmd(playcommand,"Set";);
	CurrentSongChangedMessageCommand=cmd(finishtweening;playcommand,"Set";);
};

local ScreenName = Var "LoadingScreen";

function MakeBitmapTest()
	return LoadFont(ScreenName,"StageDisplay") .. {
		InitCommand=cmd(shadowlength,0;uppercase,true;);
	};
end

for s in ivalues(Stage) do
	stages[#stages+1] = MakeBitmapTest() .. {
		SetCommand=function(self, params)
			local Stage = GAMESTATE:GetCurrentStage();
			local StageIndex = GAMESTATE:GetCurrentStageIndex();
			local screen = SCREENMAN:GetTopScreen();
			if screen and screen.GetStageStats then
				local ss = screen:GetStageStats();
				Stage = ss:GetStage();
				StageIndex = ss:GetStageIndex();
			end
			self:visible( Stage == s );
			self:settext( StageToLocalizedString(Stage) );
			self:diffuse( StageToColor(s) );
			self:strokecolor( StageToStrokeColor(s) );
		end;
	}
end

return stages;
