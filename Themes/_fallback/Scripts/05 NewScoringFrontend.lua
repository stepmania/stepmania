--[[DO NOT MODIFY THIS FILE.
You can add new scoring modes just by creating a different file and adding them to the
ScoringModes table. Replacing this file will cause additions to be lost during updates.
It's unnecessary and problematic.]]

ScoringModes={}

local function CreateOldScoringShim(name)
	local func = Scoring[name]
	return function(judg,pss,player,mode)
		judg,pss,player,mode=coroutine.yield()
		while true do
			if mode=="update" then
      if not judg.TapNoteScore:find("Mine") then func(judg,pss) end
				judg,pss,player,mode=coroutine.yield(pss:GetScore(),pss:GetCurMaxScore())
			elseif mode=="finalize" then
				return pss:GetScore(),pss:GetCurMaxScore()
			else
				error("scoring shim for mode "..name.." got an unknown state.")
			end
		end
	end
end

local ZeroIfNotFound = { __index = function() return 0 end; };

function GetDirectRadar(player)
	return GAMESTATE:GetCurrentSteps(player):GetRadarValues(player)
end

--[[----------------------------------------------------------------------------
Scoring modes begin here.
--]]----------------------------------------------------------------------------

--[[There are three operation modes:
init mode: The JudgmentCommand is invalid in this mode.
	This mode is designed to allow you to prepare your scoring mode.
update mode: all are valid.
	This lets you respond to score changes.
finalize mode: JudgementCommand invalid.
	This is designed to let you give score bonuses.
]]

ScoringModes["Oldschool"]=function(judg,pss,player,mode)
	local tscore = 0
	local tmaxscore = 1000000000
	local possibilities= { 
		['TapNoteScore_W1']=999, 
		['TapNoteScore_W2']=IsW1Allowed('TapNoteScore_W2') and 888 or 999, 
		['TapNoteScore_W3']=777,
		['TapNoteScore_W4']=555,
		['TapNoteScore_W5']=111, 
	};
	assert(mode=="init","issues")
	setmetatable(possibilities, {__index=function() return 0 end;});
	judg,pss,player,mode = coroutine.yield()
	--holy crap it's a state machine i guess!
	while true do
		if mode=="update" then
			tscore=tscore+((pss:GetCurrentCombo()*111)^1.1)+ possibilities[judg.TapNoteScore] + (judg.HoldNoteScore == 'HoldNoteScore_Held' and 777 or 0)
			judg,pss,player,mode = coroutine.yield(tscore,tmaxscore)
		end
		if mode=="finalize" then
			break
		end
	end
	return tscore,tmaxscore
end

ScoringModes['DDRMAX']=function(judge,pss,player,mode)
  --If you know Japanese and English, and can translate these comments, please help.
  --日本のコメントは、機械解釈されました。 あなたが彼らを人間を翻訳されたコメントと入れ替えるのを手伝うことができるならば、どうぞ。
	--[en] in init mode, you start here. judge will be nil.
  --[jp] initモードで、あなたがここに出発すること。judgeが、nilです
	local multLookup = {
		TapNoteScore_W1=10,
		TapNoteScore_W2=10,
		TapNoteScore_W3=5 }
	local notesToIgnore = {
		TapNoteScore_CheckpointHit=true,
		TapNoteScore_CheckpointMiss=true,
		TapNoteScore_HitMine=true,
		TapNoteScore_AvoidMine=true,
		TapNoteScore_None=true}
	local radarCategoriesAndBonuses={
		RadarCategory_Stream=20000000,
		RadarCategory_Air=10000000,
		RadarCategory_Voltage=10000000,
		RadarCategory_Chaos=10000000,
		RadarCategory_Freeze=10000000}
	setmetatable(multLookup,ZeroIfNotFound)
	local numNoteObjects = GetDirectRadar(player):GetValue('RadarCategory_TapsAndHolds')
	local base = 5000000
	local curstep = 0
	local score = 0
	local bestscore = 0
	local s = numNoteObjects<1 and 0 or ((numNoteObjects/2)*(1+numNoteObjects))
	--[en] coroutine.yield() acts somewhat like return, but when the function is run by UpdateScoreKeepers() it will start here.
  --[jp] coroutine.yield（）はいくぶん「return」のようなふりをします、しかし、機能がUpdateScoreKeepers（）に通されるとき、それはここで始まります。
	judge,pss,player,mode = coroutine.yield()
	--[en] a while loop is only one way to make the mode selection system, but it's probably the easiest.
  --[jp]「while」ループはモード選択システムを行う1つの方法だけです、しかし、それは多分最も簡単でしょう。
	while true do
		--update mode code
		if mode == "update" then
			--[en] this filters out holds and spurious judgments. DDRMAX ignores holds for scoring purposes.
			--[jp] これは、holdともっともらしい判断を除去します。 DDRMAXは、目的を記録するために、holdを無視します。
			if (not notesToIgnore[judge.TapNoteScore]) and judge.HoldNoteScore==nil then
				curstep=curstep+1
        if curstep>numNoteObjects then error "Got some things we shouldn't have" end
				score=score+(pss:GetFailed() and 1 or (multLookup[judge.TapNoteScore]*math.floor(base/s)*curstep))+((curstep==numNoteObjects and pss:FullComboOfScore('TapNoteScore_W2')) and (10*(base-(math.floor(base/s)*numNoteObjects))) or 0)
				bestscore = (curstep==numNoteObjects and 50000000 or bestscore+(10*math.floor(base/s)*curstep))
			end
			judge,pss,player,mode=coroutine.yield(score,bestscore)
		end
		if mode == "finalize" then
      --[en] this code isn't accurate, but it's good enough
			--[jp] このコードは正確でありません、しかし、それは十分によいです
			local actradar = pss:GetRadarActual()
			local possradar = pss:GetRadarPossible()
			for k,v in pairs(radarCategoriesAndBonuses) do
				local thispossradar=possradar:GetValue(k)
				if thispossradar>0 then
score=score+math.floor((actradar:GetValue(k)/thispossradar)*(thispossradar*v))
					bestscore=bestscore+(thispossradar*v)
				end
			end
			break
		end
	end
	return score,bestscore
end

ScoringModes["DDR SuperNOVA 2"]=function(judge,pss,player,mode)
	local function dTransform(number)
		return 10*math.round(number/10)
	end
	local curScore=0
	local curMaxScore=0
	local tnsMultiplier={
		TapNoteScore_W1=1,
		TapNoteScore_W2=1,
		TapNoteScore_W3=0.5,
		TapNoteScore_AvoidMine=1}
	local subtract10Points = {
		TapNoteScore_W2=IsW1Allowed('TapNoteScore_W2'),
		TapNoteScore_W3=true}
	setmetatable(tnsMultiplier, ZeroIfNotFound)
	local radar =  pss:GetRadarPossible()
	local stepScore = 1000000/math.max(radar:GetValue('RadarCategory_TapsAndHolds')+
		radar:GetValue('RadarCategory_Mines')+
		radar:GetValue('RadarCategory_Holds')+
		radar:GetValue('RadarCategory_Rolls'),1)
	--this is how one activates ScoringEngine2's built-in filter system.
	judge,pss,player,mode=coroutine.yield({{'TapNoteScore_None','TapNoteScore_CheckpointHit','TapNoteScore_CheckpointMiss'},{"HoldNoteScore_None"}})
	local mineWasHit=false
	while mode=="update" do
		if judge.TapNoteScore:find("Mine") then mineWasHit = true end
		curScore=curScore+(stepScore*(not judge.HoldNoteScore and tnsMultiplier[judge.TapNoteScore] - 
			(subtract10Points[judge.TapNoteScore] and 10 or 0) or 
			(judge.HoldNoteScore=='HoldNoteScore_Held' and 1 or 0)))
		curMaxScore=curMaxScore+stepScore
		judge,pss,player,mode=coroutine.yield(dTransform(curScore),dTransform(curMaxScore))
	end
	--We might not always be able to register mines in gameplay. Add the scores after gameplay if we couldn't.
	if not mineWasHit and radar:GetValue('RadarCategory_Mines')~=0 then
		curMaxScore=curMaxScore+(stepScore*radar:GetValue('RadarCategory_Mines'))
		curScore=curScore+(stepScore*pss:GetRadarActual():GetValue('RadarCategory_Miines'))
	end
	return dTransform(curScore),dTransform(curMaxScore)
end

ScoringModes["Billions DP"]=function(judge,pss,player,mode)
	local possibleDP=pss:GetPossibleDancePoints()
	possibleDP=possibleDP>0 and possibleDP or 1
	local curScore = 0
	local curMaxScore = 0
	while true do
		judge,pss,player,mode=coroutine.yield(curScore,curMaxScore)
		curScore = (pss:GetActualDancePoints()/possibleDP)*1000000000
		curMaxScore = (pss:GetCurrentPossibleDancePoints/possibleDP)*1000000000
		if mode=="finalize" then return curScore,curMaxScore end
	end
end

--[[----------------------------------------------------------------------------
Scoring modes end here.
--]]----------------------------------------------------------------------------
for k,v in pairs(Scoring) do
	if ScoringModes[k] == nil then
		ScoringModes[k] = CreateOldScoringShim(k)
	end
end

--if you are using the old Scoring system in the standard way, your theme will not need to be updated
local OldCallShimMt={
	__index=function(t,k,v)
		return function(judg, _)
			--sacrifice some efficiency for reliability
			InitScoreKeepers(ArgsIfPlayerJoinedOrNil(v))
			UpdateScoreKeepers(judg)
		end
	end
}

Scoring={}
setmetatable(Scoring,OldCallShimMt)

function UserPrefScoringMode()
  local baseChoices = {}
  for k,v in pairs(ScoringModes) do table.insert(baseChoices,k) end
  if next(baseChoices) == nil then UndocumentedFeature "No scoring modes available" end
	local t = {
		Name = "UserPrefScoringMode";
		LayoutType = "ShowAllInRow";
		SelectType = "SelectOne";
		OneChoiceForAllPlayers = true;
		ExportOnChange = false;
		Choices = baseChoices;
		LoadSelections = function(self, list, pn)
			if ReadPrefFromFile("UserPrefScoringMode") ~= nil then
        --Load the saved scoring mode from UserPrefs.
				local theValue = ReadPrefFromFile("UserPrefScoringMode");
				local success = false; 
        --HACK: Preview 4 took out 1st and 4th scoring. Replace with a close equivalent.
        if theValue == "DDR 1stMIX" or theValue == "DDR 4thMIX" then theValue = "Oldschool" end
        --Search the list of scoring modes for the saved scoring mode.        
				for k,v in ipairs(baseChoices) do if v == theValue then list[k] = true success = true break end end;
        --We couldn't find it, pick the first available scoring mode as a sane default.
				if success == false then list[1] = true end;
			else
        WritePrefToFile("UserPrefScoringMode", baseChoices[1]);
				list[1] = true;
			end;
		end;
		SaveSelections = function(self, list, pn)
			for k,v in ipairs(list) do if v then WritePrefToFile("UserPrefScoringMode", baseChoices[k]) break end end;
		end;
	};
	setmetatable( t, t );
	return t;
end