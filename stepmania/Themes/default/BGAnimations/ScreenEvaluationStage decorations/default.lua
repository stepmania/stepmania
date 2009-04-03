function GraphDisplay( pn )
	local t = Def.ActorFrame {
		Def.GraphDisplay {
			InitCommand=cmd(Load,"GraphDisplay";);
			BeginCommand=function(self)
				local ss = SCREENMAN:GetTopScreen():GetStageStats();
				self:Set( ss, ss:GetPlayerStageStats(pn) );
				self:player( pn );
			end
		};
	};
	return t;
end

function ComboGraph( pn )
	local t = Def.ActorFrame {
		Def.ComboGraph {
			InitCommand=cmd(Load,"ComboGraph";);
			BeginCommand=function(self)
				local ss = SCREENMAN:GetTopScreen():GetStageStats();
				self:Set( ss, ss:GetPlayerStageStats(pn) );
				self:player( pn );
			end
		};
	};
	return t;
end


local t = LoadFallbackB();

if ShowStandardDecoration("GraphDisplay") then
	for pn in ivalues(PlayerNumber) do
		t[#t+1] = StandardDecorationFromTable( "GraphDisplay" .. ToEnumShortString(pn), GraphDisplay(pn) );
	end
end

if ShowStandardDecoration("ComboGraph") then
	for pn in ivalues(PlayerNumber) do
		t[#t+1] = StandardDecorationFromTable( "ComboGraph" .. ToEnumShortString(pn), ComboGraph(pn) );
	end
end

if ShowStandardDecoration("ItsARecord") then
	for pn in ivalues(PlayerNumber) do
		-- only check if player exists, don't draw for both if one doesn't exist -aj
		if GAMESTATE:IsSideJoined(pn) then
			local t2 = Def.ActorFrame {
				BeginCommand=function(self)
					local pss = SCREENMAN:GetTopScreen():GetStageStats():GetPlayerStageStats(pn);
					local index = pss:GetMachineHighScoreIndex();
					local pSongOrCourse = GAMESTATE:GetCurrentCourse() or GAMESTATE:GetCurrentSong();
					local pSteps = GAMESTATE:GetCurrentSteps(pn);
					local hsl = PROFILEMAN:GetMachineProfile():GetHighScoreList(pSongOrCourse,pSteps);
					
					local hs = hsl:GetHighScores()[1]
					local hsName = hs:GetName();
					local hsPerc = FormatPercentScore( hs:GetPercentDP() );

					if index == 0 then
						self:GetChild("Record"):visible( true );
						self:GetChild("NoRecord"):visible( false );
					else
						self:GetChild("Record"):visible( false );
						self:GetChild("NoRecord"):visible( true );
						if hsl then
							self:GetChild("NoRecord"):settext(hsName..":\n"..hsPerc .. "%");
						else
							self:GetChild("NoRecord"):settext("");
						end
					end
				end;
				LoadFont("_sf sports night ns upright 16px") .. {
					InitCommand=cmd(name,"Record";settext,"It's a New\nRecord!!!";diffuse,color("#fffc00");strokecolor,color("#807e00");vertspacing,-2;shadowlength,0;glowshift;); 
				};
				LoadFont("common normal") .. {
					InitCommand=cmd(name,"NoRecord";strokecolor,color("#706f43");shadowlength,0;); 
				};
			}
			t[#t+1] = StandardDecorationFromTable( "ItsARecord" .. ToEnumShortString(pn), t2 );
		end
	end
end

if ShowStandardDecoration("TimingDifficulty") then
	t[#t+1] = StandardDecorationFromFile( "TimingDifficultyFrame", "TimingDifficultyFrame" );
	local t2 = LoadFont( Var "LoadingScreen", "TimingDifficultyNumber" ) .. {
		InitCommand=cmd(settext,GetTimingDifficulty(););
	};
	t[#t+1] = StandardDecorationFromTable( "TimingDifficultyNumber", t2 );
end

if ShowStandardDecoration("ModIconRows") then
	for pn in ivalues(PlayerNumber) do
		local t2 = Def.ModIconRow {
				InitCommand=cmd(Load,"ModIconRowEvaluation"..ToEnumShortString(pn),pn);
			};	
		t[#t+1] = StandardDecorationFromTable( "ModIconRow" .. ToEnumShortString(pn), t2 );
	end
end

if ShowStandardDecoration("StepsDisplay") then
	for pn in ivalues(PlayerNumber) do
		local t2 = Def.StepsDisplay {
				InitCommand=cmd(Load,"StepsDisplayEvaluation",pn;SetFromGameState,pn;);
			};	
		t[#t+1] = StandardDecorationFromTable( "StepsDisplay" .. ToEnumShortString(pn), t2 );
	end
end

if ShowStandardDecoration("StageAward") then
	for pn in ivalues(PlayerNumber) do
		t[#t+1] = StandardDecorationFromFile( "StageAward" .. ToEnumShortString(pn), "StageAward"  ) .. {
			BeginCommand=function(self)
				local ss = SCREENMAN:GetTopScreen():GetStageStats();
				self:playcommand( "Set", { StageAward = ss:GetPlayerStageStats(pn):GetStageAward() } );
			end;
		}
	end
end

for i in ivalues(EarnedExtraStage) do
	if i ~= "EarnedExtraStage_No" then
		t[#t+1] = StandardDecorationFromFile( "TryExtraStage", "Try"..ToEnumShortString(i) ) .. {
			BeginCommand=function(self)
				local ss = SCREENMAN:GetTopScreen():GetStageStats();
				self:visible( i == ss:GetEarnedExtraStage() );
			end;
		};
	end
end

return t;
