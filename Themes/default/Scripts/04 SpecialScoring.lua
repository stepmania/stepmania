--Special Scoring types.
local r = {};
--the following metatable makes any missing value in a table 0 instead of nil.
local ZeroIfNotFound = { _index = function() return 0 end; };
-----------------------------------------------------------
--DDR 1st Mix and 2nd Mix Scoring
-----------------------------------------------------------
r[1] = function(params, pss)
	local dCombo = math.floor(pss:GetCurrentCombo()/4);
	local bScore = (m^2+1) * 100;
	local multLookup = { ['TapNoteScore_W1']=3, ['TapNoteScore_W2']=3, ['TapNoteScore_W3']=1 };
	setmetatable(multLookup, ZeroIfNotFound);
	pss:SetScore(pss:GetScore()+(bScore*multLookup[params.TapNoteScore]));
end;
-----------------------------------------------------------
--DDR 3rd Mix and USA Scoring
-----------------------------------------------------------
r[2] = function(params, pss)
	local totalSteps = GAMESTATE:GetCurrentSteps(params.Player):GetRadarValues():GetValue('RadarCategory_TapsAndHolds');
	local baseScore = math.round(1000000/((totalSteps*(totalSteps+1))/2));
	local curStep = pss:GetRadarActual:GetValue('RadarCategory_TapsAndHolds');
	local endBonus = (curStep == totalSteps) and (1000000-math.round(1000000/(totalSteps*(totalSteps+1)/2))*(totalSteps*(totalSteps+1)/2)) or 0;
	local multLookup = { ['TapNoteScore_W1']=10, ['TapNoteScore_W2']=10, ['TapNoteScore_W3']=5 };
	setmetatable(multLookup, ZeroIfNotFound);
	local thisScore = multLookup[params.TapNoteScore] * (baseScore * curStep + endBonus);
	pss:SetScore(pss:GetScore()+thisScore);
end;
-----------------------------------------------------------
--DDR 4th Mix/Extra Mix/Konamix/GB3/DDRPC Scoring
-----------------------------------------------------------
r[3] = function(params, pss)
	local scoreLookupTable = { ['TapNoteScore_W1']=777, ['TapNoteScore_W2']=777, ['TapNoteScore_W3']=555 };
	setmetatable(scoreLookupTable, ZeroIfNotFound); 
	local comboBonusForThisStep = pss:GetCurrentCombo()*333;
	pss:SetScore(pss:GetScore()+scoreLookupTable[params.TapNoteScore]+comboBonusForThisStep);
end;
-----------------------------------------------------------
--DDR SuperNOVA(-esque) scoring
-----------------------------------------------------------
r[4] = function(params, pss)
	pss:SetScore(math.round(pss:GetActualDancePoints()/pss:GetPossibleDancePoints()*1000000));
end;
SpecialScoring = r;
