local stages = Def.ActorFrame {
	BeginCommand=cmd(playcommand,"Set";);
	CurrentSongChangedMessageCommand=cmd(finishtweening;playcommand,"Set";);
};

local ScreenName = Var "LoadingScreen";

function MakeBitmapTest()
	return LoadFont(ScreenName,"StageDisplay") .. {

	};
end
if not PREFSMAN:GetPreference("EventMode") then
	for s in ivalues(Stage) do
		stages[#stages+1] = MakeBitmapTest() .. {
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
				if Stage == 'Stage_Event' then
					self:settext( StageToLocalizedString(Stage) .. " Mode" );
				else
					self:settext( StageToLocalizedString(Stage) .. " Stage" );
				end
				self:diffuse( StageToColor(s) );
				self:diffusebottomedge( ColorMidTone(StageToColor(s)) );
				self:strokecolor( cStageOutlineColor );
			end;
		}
	end
else
	stages[#stages+1] = MakeBitmapTest() .. {
		SetCommand=function(self,params)
			local Stage = GAMESTATE:GetCurrentStageIndex();
			self:settextf( "Stage %03i", Stage);
				self:diffuse( StageToColor('Stage_1st') );
				self:diffusebottomedge( ColorMidTone(StageToColor('Stage_1st')) );
				self:strokecolor( Colors.Alpha( ColorDarkTone(StageToColor('Stage_1st')), 0.75) );
		end;
	}
end

return stages;
