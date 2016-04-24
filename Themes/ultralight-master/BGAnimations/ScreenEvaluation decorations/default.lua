local t = LoadFallbackB();
--[[--------------------------------------------------------------------------]]
t[#t+1] = StandardDecorationFromFileOptional("StageDisplay","StageDisplay");

local function GraphDisplay( pn )
	local t = Def.ActorFrame {
		Def.GraphDisplay {
			InitCommand=cmd(Load,"GraphDisplay";);
			BeginCommand=function(self)
				local ss = SCREENMAN:GetTopScreen():GetStageStats();
				self:Set( ss, ss:GetPlayerStageStats(pn) );
			end
		};
	};
	return t;
end

if ShowStandardDecoration("GraphDisplay") then
	for pn in ivalues(PlayerNumber) do
		if IsPlayerValid(pn) then
			t[#t+1] = StandardDecorationFromTable( "GraphDisplay" .. ToEnumShortString(pn), GraphDisplay(pn) )
		end;
	end
end
--[[--------------------------------------------------------------------------]]
local function ComboGraph( pn )
	local t = Def.ActorFrame {
		Def.ComboGraph {
			InitCommand=cmd(Load,"ComboGraph"..ToEnumShortString(pn););
			BeginCommand=function(self)
				local ss = SCREENMAN:GetTopScreen():GetStageStats();
				self:Set( ss, ss:GetPlayerStageStats(pn) );
			end
		};
	};
	return t;
end

if ShowStandardDecoration("ComboGraph") then
	for pn in ivalues(PlayerNumber) do
		if IsPlayerValid(pn) then
			t[#t+1] = StandardDecorationFromTable( "ComboGraph" .. ToEnumShortString(pn), ComboGraph(pn) );
		end;
	end
end
--[[--------------------------------------------------------------------------]]

t[#t+1] = StandardDecorationFromFileOptional("SelectionInfo","SelectionInfo");

if ShowStandardDecoration("StepsDisplay") then
	for pn in ivalues(PlayerNumber) do
		if IsPlayerValid(pn) then
			local t2 = Def.StepsDisplay {
					InitCommand=cmd(Load,"StepsDisplayEvaluation",pn;SetFromGameState,pn;);
				};
			t[#t+1] = StandardDecorationFromTable( "StepsDisplay" .. ToEnumShortString(pn), t2 );
		end;
	end
end
--[[
for pn in ivalues(GAMESTATE:GetHumanPlayers()) do
	local MetricsName = "StageAward" .. PlayerNumberToString(pn);
	t[#t+1] = LoadActor( THEME:GetPathG(Var "LoadingScreen", "StageAward"), pn ) .. {
		InitCommand=function(self) 
			self:player(pn); 
			self:name(MetricsName); 
			ActorUtil.LoadAllCommandsAndSetXY(self,Var "LoadingScreen"); 
		end;
		BeginCommand=cmd(playcommand,"Set");
		SetCommand=function(self)
			local tStats = THEME:GetMetric(Var "LoadingScreen", "Summary") and STATSMAN:GetAccumPlayedStageStats() or STATSMAN:GetCurStageStats();
			tStats = tStats:GetPlayerStageStats(pn);
			if tStats:GetStageAward() then
				self:settext( THEME:GetString( "StageAward", ToEnumShortString( tStats:GetStageAward() ) ) );
			else
				self:settext( "" );
			end
		end;
	};
end

for pn in ivalues(GAMESTATE:GetHumanPlayers()) do
	local MetricsName = "PeakComboAward" .. PlayerNumberToString(pn);
	t[#t+1] = LoadActor( THEME:GetPathG(Var "LoadingScreen", "PeakComboAward"), pn ) .. {
		InitCommand=function(self) 
			self:player(pn); 
			self:name(MetricsName); 
			ActorUtil.LoadAllCommandsAndSetXY(self,Var "LoadingScreen"); 
		end;
		BeginCommand=cmd(playcommand,"Set");
		SetCommand=function(self)
			local tStats = THEME:GetMetric(Var "LoadingScreen", "Summary") and STATSMAN:GetAccumPlayedStageStats() or STATSMAN:GetCurStageStats();
			tStats = tStats:GetPlayerStageStats(pn);
			if tStats:GetPeakComboAward() then
				self:settext( THEME:GetString( "PeakComboAward", ToEnumShortString( tStats:GetPeakComboAward() ) ) );
			else
				self:settext( "" );
			end
		end;
	};
end
--]]

-- percentages
local judgeLines = {
	'JudgmentLine_W1',
	'JudgmentLine_W2',
	'JudgmentLine_W3',
	'JudgmentLine_W4',
	'JudgmentLine_W5',
	'JudgmentLine_Miss'
};
for i=1,#judgeLines do
	local judge = ToEnumShortString(judgeLines[i]);
	local tns = 'TapNoteScore_'..judge
	for pn in ivalues(PlayerNumber) do
		if ShowStandardDecoration("JudgmentLine"..judge) then
			local metric = judge.."Number"..ToEnumShortString(pn);
			local xPos = THEME:GetMetric(Var "LoadingScreen",metric.."X")+25;
			local yPos = THEME:GetMetric(Var "LoadingScreen",metric.."Y")+4;
			t[#t+1] = LoadFont("Common numbers")..{
				Name=judge.."Percent"..ToEnumShortString(pn); -- e.g. "W1PercentP1"
				InitCommand=cmd(x,xPos;y,yPos;zoom,0.45;diffuse,TapNoteScoreToColor(tns););
				BeginCommand=function(self)
					self:visible(GAMESTATE:IsPlayerEnabled(pn))
					self:addx(pn == PLAYER_1 and 52 or -75);
					self:halign(1)
					local playerStageStats = STATSMAN:GetCurStageStats():GetPlayerStageStats(pn)
					local count = playerStageStats:GetPercentageOfTaps(tns);
					local p = tonumber(string.format("%.00f",count*100));
					self:settext( p.."%" );
				end;
				OffCommand=THEME:GetMetric(Var "LoadingScreen",metric.."OffCommand");
			};
		end;
	end
end

return t