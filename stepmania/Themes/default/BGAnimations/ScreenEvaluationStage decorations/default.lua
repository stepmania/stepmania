function GraphDisplay( pn )
	local t = Def.ActorFrame {
		Def.GraphDisplay {
			InitCommand=cmd(Load,"GraphDisplay";);
			BeginCommand=function(self)
				local screen = SCREENMAN:GetTopScreen();
				local ss = screen:GetStageStats();
				self:Set( ss, ss:GetPlayerStageStats(pn) );
			end,
		};
	};
	return t;
end

function ComboGraph( pn )
	local t = Def.ActorFrame {
		Def.ComboGraph {
			InitCommand=cmd(Load,"ComboGraph";);
			BeginCommand=function(self)
				local screen = SCREENMAN:GetTopScreen();
				local ss = screen:GetStageStats();
				self:Set( ss, ss:GetPlayerStageStats(pn) );
			end,
		};
	};
	return t;
end


local t = LoadFallbackB();

if ShowStandardDecoration("GraphDisplay") then
	for pn in ivalues(PlayerNumber) do
		t[#t+1] = StandardDecorationFromTable( "GraphDisplay" .. PlayerNumber:ToString()[pn], GraphDisplay(pn) );
	end
end

if ShowStandardDecoration("ComboGraph") then
	for pn in ivalues(PlayerNumber) do
		t[#t+1] = StandardDecorationFromTable( "ComboGraph" .. PlayerNumber:ToString()[pn], ComboGraph(pn) );
	end
end

t[#t+1] = StandardDecorationFromFile( "TimingDifficultyFrame", "TimingDifficultyFrame" );
t[#t+1] = LoadFont( Var "LoadingScreen", "TimingDifficultyNumber" ) .. {
	InitCommand=function(self) self:name("TimingDifficultyNumber"); self:settext(GetTimingDifficulty()); ActorUtil.LoadAllCommandsAndSetXY(self,Var "LoadingScreen"); end;
};


if ShowStandardDecoration("ModIconRows") then
	for pn in ivalues(PlayerNumber) do
		local t2 = Def.ModIconRow {
				InitCommand=cmd(Load,"ModIconRowEvaluation"..PlayerNumber:ToString()[pn],pn);
			};	
		t[#t+1] = StandardDecorationFromTable( "ModIconRow" .. PlayerNumber:ToString()[pn], t2 );
	end
end

if ShowStandardDecoration("StepsDisplay") then
	for pn in ivalues(PlayerNumber) do
		local t2 = Def.StepsDisplay {
				InitCommand=cmd(Load,"StepsDisplayEvaluation",pn;SetFromGameState,pn;);
			};	
		t[#t+1] = StandardDecorationFromTable( "StepsDisplay" .. PlayerNumber:ToString()[pn], t2 );
	end
end

return t;