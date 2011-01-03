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
	pss:SetScore(pss:GetScore()+scoreLookupTable[params.TapNoteScore]+comboBonusForThisStep);
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
--Radar Master
-----------------------------------------------------------
r['[SSC] Radar Master'] = function(params, pss)
	local masterTable = {
		['RadarCategory_Stream'] = 0,
		['RadarCategory_Voltage'] = 0,
		['RadarCategory_Air'] = 0,
		['RadarCategory_Freeze'] = 0,
		['RadarCategory_Chaos'] = 0
	};
	local totalRadar = 0;
	local finalScore = 0;
	for k,v in ipairs(masterTable) do
		local firstRadar = pss:GetRadarPossible():GetValue(k);
		if firstRadar == 0 then
			masterTable[k] = nil;
		else
			v = pss:GetRadarPossible():GetValue(k);
			totalRadar = totalRadar + v[1];
		end;
	end;
	for k,v in ipairs(masterTable) do
		local curPortion = pss:GetRadarActual():GetValue(k) / v;
		finalScore = finalScore + curPortion*(500000000*(v/totalRadar));
	end;
end;	 
SpecialScoring = r;
