--Special Scoring types.
local r = {};
--the following metatable makes any missing value in a table 0 instead of nil.
local ZeroIfNotFound = { __index = function() return 0 end; };
-----------------------------------------------------------
--DDR 1st Mix and 2nd Mix Scoring
-----------------------------------------------------------
r['DDR 1stMIX'] = function(params, pss)
	local dCombo = math.floor((pss:GetCurrentCombo()+1)/4);
	local bScore = (dCombo^2+1) * 100;
	local multLookup = { ['TapNoteScore_W1']=3, ['TapNoteScore_W2']=3, ['TapNoteScore_W3']=1 };
	setmetatable(multLookup, ZeroIfNotFound);
	pss:SetScore(pss:GetScore()+(bScore*multLookup[params.TapNoteScore]));
end;
-----------------------------------------------------------
--DDR 4th Mix/Extra Mix/Konamix/GB3/DDRPC Scoring
-----------------------------------------------------------
r['DDR 4thMIX'] = function(params, pss)
	local scoreLookupTable = { ['TapNoteScore_W1']=777, ['TapNoteScore_W2']=777, ['TapNoteScore_W3']=555 };
	setmetatable(scoreLookupTable, ZeroIfNotFound); 
	local comboBonusForThisStep = (pss:GetCurrentCombo()+1)*333;
	pss:SetScore(pss:GetScore()+scoreLookupTable[params.TapNoteScore]+(scoreLookupTable[params.TapNoteScore] and comboBonusForThisStep or 0));
end;
-----------------------------------------------------------
--DDR SuperNOVA(-esque) scoring
-----------------------------------------------------------
r['DDR SuperNOVA'] = function(params, pss)
	local dp = pss:GetPossibleDancePoints();
	if dp == 0 then pss:SetScore(0) return nil end
	pss:SetScore(math.round((pss:GetActualDancePoints()/dp)*1000000));
end;
-----------------------------------------------------------
--DDR SuperNOVA 2(-esque) scoring
-----------------------------------------------------------
r['DDR SuperNOVA 2'] = function(params, pss)
	local dp = pss:GetPossibleDancePoints();
	if dp == 0 then pss:SetScore(0) return nil end
	pss:SetScore(math.round((pss:GetActualDancePoints()/dp)*100000)*10);
end;
-----------------------------------------------------------
--Radar Master (doesn't work in 1.2, disabled)
--don't try to "fix it up", either. you *cannot* make it work in 1.2.
-----------------------------------------------------------
--[[r['[SSC] Radar Master'] = function(params, pss)
	local masterTable = {
		['RadarCategory_Stream'] = 0,
		['RadarCategory_Voltage'] = 0,
		['RadarCategory_Air'] = 0,
		['RadarCategory_Freeze'] = 0,
		['RadarCategory_Chaos'] = 0
	};
	local totalRadar = 0;
	local finalScore = 0;
	for k,v in pairs(masterTable) do
		local firstRadar = GAMESTATE:GetCurrentSteps(params.Player):GetRadarValues(params.Player):GetValue(k);
		if firstRadar == 0 then
			masterTable[k] = nil;
		else
			masterTable[k] = firstRadar;
			totalRadar = totalRadar + firstRadar;
		end;
	end;
	--two loops are needed because we need to calculate totalRadar
	--to actually calculate any part of the score
	for k,v in pairs(masterTable) do
		local curPortion = pss:GetRadarActual():GetValue(k) / v;
		finalScore = finalScore + curPortion*(500000000*(v/totalRadar));
	end;
	pss:SetScore(finalScore);
end;]]
------------------------------------------------------------
--Marvelous Incorporated Grading System (or MIGS for short)
--basically like DP scoring with locked DP values
------------------------------------------------------------
r['MIGS'] = function(params,pss)
	local curScore = 0;
	local tapScoreTable = { ['TapNoteScore_W1'] = 3, ['TapNoteScore_W2'] = 2, ['TapNoteScore_W3'] = 1, ['TapNoteScore_W5'] = -4, ['TapNoteScore_Miss'] = -8 };
	for k,v in pairs(tapScoreTable) do
		curScore = curScore + ( pss:GetTapNoteScores(k) * v );
	end;
	curScore = curScore + ( pss:GetHoldNoteScores('HoldNoteScore_Held') * 6 );
	pss:SetScore(clamp(curScore,0,math.huge));
end;
SpecialScoring = r;
