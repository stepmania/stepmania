--internals table
local Shared = {};
--Special Scoring types.
local r = {};
local DisabledScoringModes = { '[SSC] Radar Master' };
--the following metatable makes any missing value in a table 0 instead of nil.
local ZeroIfNotFound = { __index = function() return 0 end; };

-- Retrieve the amount of taps/holds/rolls involved. Used for some formulas.
function GetTotalItems(radars)
	return radars:GetValue('RadarCategory_TapsAndHolds') 
		+ radars:GetValue('RadarCategory_Holds') 
		+ radars:GetValue('RadarCategory_Rolls')
		+ radars:GetValue('RadarCategory_Lifts');
end;

-- Determine whether marvelous timing is to be considered.
function IsW1Allowed(tapScore)
	return tapScore == 'TapNoteScore_W2'
		and (PREFSMAN:GetPreference("AllowW1") ~= 'AllowW1_Never' 
		or not (GAMESTATE:IsCourseMode() and 
		PREFSMAN:GetPreference("AllowW1") == 'AllowW1_CoursesOnly'));
end;

-- Get the radar values directly. The individual steps aren't used much.
function GetDirectRadar(player)
	return GAMESTATE:GetCurrentSteps(player):GetRadarValues(player);
end;

-----------------------------------------------------------
--DDR 1st Mix and 2nd Mix Scoring
-----------------------------------------------------------
r['DDR 1stMIX'] = function(params, pss)
	local dCombo = math.floor((pss:GetCurrentCombo()+1)/4);
	local bScore = (dCombo^2+1) * 100;
	local multLookup = { ['TapNoteScore_W1']=3, ['TapNoteScore_W2']=3, ['TapNoteScore_W3']=1 };
	setmetatable(multLookup, ZeroIfNotFound);
	--if score increases above the boundaries of a 32-bit signed
	--(about 2.15 billion), it stops increasing. Conveniently,
	--1st Mix clamped score as well.
	pss:SetScore(clamp(pss:GetScore()+(bScore*multLookup[params.TapNoteScore]),0,999999999));
end;
-----------------------------------------------------------
--DDR 4th Mix/Extra Mix/Konamix/GB3/DDRPC Scoring
-----------------------------------------------------------
r['DDR 4thMIX'] = function(params, pss)
	local scoreLookupTable = { ['TapNoteScore_W1']=777, ['TapNoteScore_W2']=777, ['TapNoteScore_W3']=555 };
	setmetatable(scoreLookupTable, ZeroIfNotFound); 
	local comboBonusForThisStep = (pss:GetCurrentCombo()+1)*333;
	pss:SetScore(clamp(pss:GetScore()+scoreLookupTable[params.TapNoteScore]+(scoreLookupTable[params.TapNoteScore] and comboBonusForThisStep or 0),0,999999999));
end;
-----------------------------------------------------------
--DDR MAX2/Extreme Scoring
-----------------------------------------------------------
r['DDR Extreme'] = function(params, pss)
	local judgmentBase = {
		['TapNoteScore_W1'] = 10,
		['TapNoteScore_W2'] = 9,
		['TapNoteScore_W3'] = 5
	};
	setmetatable(judgmentBase, ZeroIfNotFound);
	local steps = GAMESTATE:GetCurrentSteps(params.Player);
	local radarValues = steps:GetRadarValues(params.Player);
	local baseScore = (steps:IsAnEdit() and 
		5 or steps:GetMeter()) * 1000000;
	local totalItems = GetTotalItems(radarValues);
	local singleStep = (1 + totalItems) * totalItems / 2;
	if (not Shared.CurrentStep) then
		Shared.CurrentStep = 0
	end;
	Shared.CurrentStep = Shared.CurrentStep + 1;
	local stepLast = math.floor(baseScore / singleStep) * (Shared.CurrentStep);
	local judgeScore = 0;
	if (params.HoldNoteScore == 'HoldNoteScore_Held') then
		judgeScore = judgmentBase['TapNoteScore_W1'];
	else
		judgeScore = judgmentBase[params.TapNoteScore];
		if (IsW1Allowed(params.TapNoteScore)) then
			judgeScore = judgmentBase['TapNoteScore_W1'];
		end;
	end;
	local stepScore = judgeScore * stepLast;
	if (Shared.CurrentStep >= totalItems) then -- Just in case.
		Shared.CurrentStep = 0; -- Reset for the next song.
	end;
	pss:SetScore(pss:GetScore() + stepScore);
end;
-----------------------------------------------------------
--DDR SuperNOVA(-esque) scoring
-----------------------------------------------------------
r['DDR SuperNOVA'] = function(params, pss)
	local multLookup = { ['TapNoteScore_W1'] = 1, ['TapNoteScore_W2'] = 1, ['TapNoteScore_W3'] = 0.5 };
	setmetatable(multLookup, ZeroIfNotFound);
	local radarValues = GetDirectRadar(params.Player);
	local totalItems = GetTotalItems(radarValues); 
	local buildScore = (10000000 / totalItems * multLookup[params.TapNoteScore]) + (10000000 / totalItems * (params.HoldNoteScore == 'HoldNoteScore_Held' and 1 or 0));
	pss:SetScore(pss:GetScore() + math.round(buildScore));
end;
-----------------------------------------------------------
--DDR SuperNOVA 2(-esque) scoring
-----------------------------------------------------------
r['DDR SuperNOVA 2'] = function(params, pss)
	local multLookup = { ['TapNoteScore_W1'] = 1, ['TapNoteScore_W2'] = 1, ['TapNoteScore_W3'] = 0.5 };
	setmetatable(multLookup, ZeroIfNotFound);
	local radarValues = GetDirectRadar(params.Player);
	local totalItems = GetTotalItems(radarValues); 
	local buildScore = (100000 / totalItems * multLookup[params.TapNoteScore] - (IsW1Allowed(params.TapNoteScore) and 10 or 0)) + (100000 / totalItems * (params.HoldNoteScore == 'HoldNoteScore_Held' and 1 or 0));
	pss:SetScore(pss:GetScore() + (math.round(buildScore) * 10));
end;
-----------------------------------------------------------
--Radar Master (doesn't work in 1.2.1, disabled)
--don't try to "fix it up", either. you *cannot* make it work in 1.2.1.
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
	for k,v in pairs(masterTable) do
		local firstRadar = GetDirectRadar(params.Player):GetValue(k);
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
end;
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
SpecialScoring = {};
setmetatable(SpecialScoring, { 
	__metatable = { "Letting you change the metatable sort of defeats the purpose." };
	__index = function(tbl, key)
			for v in ivalues(DisabledScoringModes) do
				if key == v then return r['DDR 1stMIX']; end;
			end;
			return r[key];
		end;
	}
);
