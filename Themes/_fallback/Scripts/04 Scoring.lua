--internals table
local Shared = {};
--Special Scoring types.
local r = {};
local DisabledScoringModes = { '[SSC] Radar Master' };
--the following metatable makes any missing value in a table 0 instead of nil.
local ZeroIfNotFound = { __index = function() return 0 end; };

-- Retrieve the amount of taps/holds/rolls involved. Used for some formulas.
function GetTotalItems(radars)
	local total = radars:GetValue('RadarCategory_TapsAndHolds')
	total = total + radars:GetValue('RadarCategory_Holds') 
	total = total + radars:GetValue('RadarCategory_Rolls')
	total = total + radars:GetValue('RadarCategory_Lifts')
	return total
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
	local multLookup = 
	{ 
		['TapNoteScore_W1']=3,
		['TapNoteScore_W2']=3,
		['TapNoteScore_W3']=1
	};
	setmetatable(multLookup, ZeroIfNotFound);
	--if score increases above the boundaries of a 32-bit signed
	--(about 2.15 billion), it stops increasing. Conveniently,
	--1st Mix clamped score as well.
	local capScore = 999999999;
	local bestScore = bScore * multLookup['TapNoteScore_W1'];
	local localScore = bScore * multLookup[params.TapNoteScore];
	pss:SetCurMaxScore(clamp(pss:GetCurMaxScore()+(bestScore),0,capScore));
	pss:SetScore(clamp(pss:GetScore()+(localScore),0,capScore));
	
end;
-----------------------------------------------------------
--DDR 4th Mix/Extra Mix/Konamix/GB3/DDRPC Scoring
-----------------------------------------------------------
r['DDR 4thMIX'] = function(params, pss)
	local scoreLookupTable =
	{ 
		['TapNoteScore_W1']=777, 
		['TapNoteScore_W2']=777, 
		['TapNoteScore_W3']=555 
	};
	setmetatable(scoreLookupTable, ZeroIfNotFound);
	-- TODO: Modify this so that current max assumes full combo?
	local comboBonusForThisStep = (pss:GetCurrentCombo()+1)*333;
	local capScore = 999999999;
	local bestPoints = scoreLookupTable['TapNoteScore_W1'];
	local bestCombo = bestPoints and comboBonusForThisStep or 0;
	pss:SetCurMaxScore(clamp(pss:GetCurMaxScore()+bestPoints+bestCombo,0,capScore));
	local localPoints = scoreLookupTable[params.TapNoteScore];
	local localCombo = localPoints and comboBonusForThisStep or 0;
	pss:SetScore(clamp(pss:GetScore()+localPoints+localCombo,0,capScore));
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
	local meter = steps:GetMeter();
	if (steps:IsAnEdit()) then
		meter = 5;
	elseif (meter < 1) then
		meter = 1;
	elseif (meter > 10) then
		meter = 10;
	end;
	local baseScore = meter * 1000000;
	if (GAMESTATE:GetCurrentSong():IsMarathon()) then
		baseScore = baseScore * 3;
	elseif (GAMESTATE:GetCurrentSong():IsLong()) then
		baseScore = baseScore * 2;
	end;
	local totalItems = GetTotalItems(radarValues);
	local singleStep = (1 + totalItems) * totalItems / 2;
	if (not Shared.CurrentStep) then
		Shared.CurrentStep = {};
		Shared.CurrentStep["P1"] = 0;
		Shared.CurrentStep["P2"] = 0;
	end;
	local pn = PlayerNumberToString(params.Player);
	Shared.CurrentStep[pn] = Shared.CurrentStep[pn] + 1;
	local stepValue = math.floor(baseScore /singleStep);
	local stepLast = stepValue * Shared.CurrentStep[pn];
	pss:SetCurMaxScore(pss:GetCurMaxScore() + 
		(stepLast * judgmentBase['TapNoteScore_W1']));
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
	pss:SetScore(pss:GetScore() + stepScore);
	if (Shared.CurrentStep[pn] >= totalItems) then -- Just in case.
		-- TODO: Implement the bonus for the last step?
		Shared.CurrentStep[pn] = 0; -- Reset for the next song.
	end;
end;

-----------------------------------------------------------
--HYBRID Scoring, contributed by @waiei
-----------------------------------------------------------
local hyb_Steps=0;
r['HYBRID'] = function(params, pss)
	local multLookup =
	{
		['TapNoteScore_W1'] = 10,
		['TapNoteScore_W2'] = 9,
		['TapNoteScore_W3'] = 5
	};
	setmetatable(multLookup, ZeroIfNotFound);
	local radarValues = GetDirectRadar(params.Player);
	-- [ja] GetTotalItems(radarValues) に Liftも含まれているので 引いておく
	local totalItems = GetTotalItems(radarValues) - radarValues:GetValue('RadarCategory_Lifts');
	-- 1+2+3+...+totalItems value/の値
	local sTotal = (totalItems+1)*totalItems/2;
	-- [en] Score for one song
	-- [ja] 1ステップあたりのスコア
	local sOne = math.floor(100000000/sTotal);
	-- [ja] 端数は最後の1ステップで加算するのでその値を取得
	local sLast = 100000000-(sOne*sTotal);

	-- [ja] スコアが0の時に初期化
	if pss:GetScore()==0 then
		hyb_Steps=0;
	end;

	-- [ja] 現在のステップ数
	hyb_Steps=hyb_Steps+1;
	pss:SetCurMaxScore(pss:GetCurMaxScore()+1);
	-- [en] current score
	-- [ja] 今回加算するスコア（W1の時）
	local vScore = sOne*hyb_Steps;
	pss:SetCurMaxScore(pss:GetCurMaxScore()+vScore);
	-- [ja] 判定によって加算量を変更
	if (params.HoldNoteScore == 'HoldNoteScore_Held') then
		vScore = vScore;
	else
		if (params.HoldNoteScore == 'HoldNoteScore_LetGo') then
			vScore = 0;
		else
			vScore = vScore*multLookup[params.TapNoteScore]/10;
		end;
	end;
	pss:SetScore(pss:GetScore()+vScore);

	-- [ja] 最後の1ステップの場合、端数を加算する
	if ((vScore > 0) and (hyb_Steps == totalItems)) then
		pss:SetScore(pss:GetScore()+sLast);
	end;
end;

-----------------------------------------------------------
--DDR SuperNOVA(-esque) scoring
-----------------------------------------------------------
r['DDR SuperNOVA'] = function(params, pss)
	local multLookup =
	{
		['TapNoteScore_W1'] = 1,
		['TapNoteScore_W2'] = 1,
		['TapNoteScore_W3'] = 0.5
	};
	setmetatable(multLookup, ZeroIfNotFound);
	local radarValues = GetDirectRadar(params.Player);
	local totalItems = GetTotalItems(radarValues); 
	local base = 10000000 / totalItems;
	local hold = base * (params.HoldNoteScore == 'HoldNoteScore_Held' and 1 or 0);
	local maxScore = (base * multLookup['TapNoteScore_W1']) + hold;
	pss:SetCurMaxScore(pss:GetCurMaxScore() + math.round(maxScore));
	local buildScore = (base * multLookup[params.TapNoteScore]) + hold;
	pss:SetScore(pss:GetScore() + math.round(buildScore));
end;
-----------------------------------------------------------
--DDR SuperNOVA 2(-esque) scoring
-----------------------------------------------------------
local sn2tmp_Sub=0;
local sn2tmp_Score=0;
local sn2tmp_Steps=0;
r['DDR SuperNOVA 2'] = function(params, pss)
	local multLookup =
	{
		['TapNoteScore_W1'] = 10,
		['TapNoteScore_W2'] = 10,
		['TapNoteScore_W3'] = 5
	};
	setmetatable(multLookup, ZeroIfNotFound);
	local radarValues = GetDirectRadar(params.Player);
	-- [ja] GetTotalItems(radarValues) に Liftも含まれているので 引いておく
	local totalItems = GetTotalItems(radarValues) - radarValues:GetValue('RadarCategory_Lifts');
	-- [ja] 0除算対策（しなくても動作するけど満点になっちゃうんで）
	if totalItems <= 0 then
		totalItems=1
	end;

	-- [ja] スコアが0の時に初期化
	if pss:GetScore() == 0 then
		sn2tmp_Sub=0;
		sn2tmp_Score=0;
		sn2tmp_Steps=0;
	end;

	-- [ja] maxAdd は 加算する最高点を 10 とした時の値（つまり、10=100% / 5=50%）
	local maxAdd = 0;
	-- [ja] O.K.判定時は問答無用で満点
	if params.HoldNoteScore == 'HoldNoteScore_Held' then
		maxAdd = 10;
	else
		-- [ja] N.G.判定時は問答無用で0点
		if params.HoldNoteScore == 'HoldNoteScore_LetGo' then
			maxAdd = 0;
		-- [ja] それ以外ということは、ロングノート以外の判定である
		else
			maxAdd = multLookup[params.TapNoteScore];
			if (params.TapNoteScore == 'TapNoteScore_W2') or (params.TapNoteScore=='TapNoteScore_W3') then
			-- [ja] W2とW3の数を記録
				sn2tmp_Sub=sn2tmp_Sub+1;
			end;
		end
	end;
	sn2tmp_Score=sn2tmp_Score+maxAdd;
	-- [ja] 踏み踏みしたステップ数
	sn2tmp_Steps=sn2tmp_Steps+1;
	-- [ja] 現時点での、All W1判定の時のスコア
	pss:SetCurMaxScore(math.floor(10000*sn2tmp_Steps/totalItems) * 10);

	-- [ja] 計算して代入
	pss:SetScore((math.floor(10000*sn2tmp_Score/totalItems) * 10) - (sn2tmp_Sub*10) );
end;
-----------------------------------------------------------
--Radar Master (disabled; todo: get this working with StepMania 5)
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
	local tapScoreTable = 
	{ 
		['TapNoteScore_W1'] = 3,
		['TapNoteScore_W2'] = 2,
		['TapNoteScore_W3'] = 1,
		['TapNoteScore_W5'] = -4,
		['TapNoteScore_Miss'] = -8
	};
	for k,v in pairs(tapScoreTable) do
		curScore = curScore + ( pss:GetTapNoteScores(k) * v );
	end;
  curScore = math.max(0,curScore + ( pss:GetHoldNoteScores('HoldNoteScore_Held') * 6 ));
end;

--------------------------------------------------------------
--1bilDP scoring because I can.
--------------------------------------------------------------
r['1bilDP']= function(params,pss)
  local score = math.floor((pss:GetActualDancePoints()/pss:GetPossibleDancePoints())*1000000000)
  pss:SetScore(score)
  pss:SetCurMaxScore(score)
end
-------------------------------------------------------------------------------
-- Formulas end here.
for v in ivalues(DisabledScoringModes) do r[v] = nil end
Scoring = r
