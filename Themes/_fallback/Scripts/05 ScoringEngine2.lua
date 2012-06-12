--ScoringEngine2 by FSX v1.43
--essentially a really chunky layer to make SetScore elegant(ish)
--arguments for scorekeepers: JudgmentMessageCommand,PlayerStageStats,PlayerNumber,State
--[[changelog
v1.43 12 Jun 2012
*Remove a log that could create a lot of log noise
in certain situations
v1.42 21 May 2012
*Add more type checking.
*Don't crash on missing modes.
v1.41 6 May 2012
*Not really sure.
*Included in SM5 for alpha 3.
v1.4 18 Apr 2012
*Support for judgment filtering.
*Minor changes.
*First public release (in SE2 Dev Kit) (didn't happen)
v1.3 17 Apr 2012
*First complete release.
*Error reporting in FinalizeScoreKeepers()
*Bug fixes.]]
--scoring modes written for the old ad-hoc system don't work, but don't cause crashes either.
--SM5 has some glue code for them, though
local stageKey = nil
local ReadiedScoreKeepers = {}
local JudgmentFilters = {}

local function CoroutineIsGood(cr)
	return coroutine.status(cr)=="suspended"
end

function GetNumReadiedScoreKeepers()
	return table.itemcount(ReadiedScoreKeepers)
end

--InitScoreKeepers(string p1Mode, string p2Mode)
--Both are optional.
function InitScoreKeepers(p1Mode, p2Mode)
	local newStgK = {pcall(GameState.GetStageSeed,GAMESTATE)}
	assert(newStgK[1], "it's too early to initialize the ScoreKeepers, the GameState isn't ready yet")
	if newStgK[2] == stageKey then
		return true
	end
	if p1Mode~=nil then
		assert(type(p1Mode)=="string", "only pass string or nil")
	end
	if p2Mode~=nil then
		assert(type(p2Mode)=="string", "only pass string or nil")
	end
	ReadiedScoreKeepers={}
	JudgmentFilters={}
	stageKey = newStgK
	local intab = {}
	if p1Mode ~= nil then
		intab.P1 = p1Mode
	end
	if p2Mode ~= nil then
		intab.P2 = p2Mode
	end
	assert(next(intab), "InitScoreKeepers didn't get any args")
	for k,v in pairs(intab) do
		if not ScoringModes[v] then
			Warn(k.."'s scoring mode \""..v.."\" doesn't exist.")
		else
			ReadiedScoreKeepers[k] = coroutine.create(ScoringModes[v])
			--call each scoring mode one time in init mode
			checks={coroutine.resume(ReadiedScoreKeepers[k],nil,STATSMAN:GetCurStageStats():GetPlayerStageStats("PlayerNumber_"..k), "PlayerNumber_"..k, "init")}
			if not CoroutineIsGood(ReadiedScoreKeepers[k]) then SCREENMAN:SystemMessage("scoring init error: "..checks[2]) return false end
			--very very very very safe filtering rule load code
			if type(checks[2])=="table" then
				if type(checks[2][1])..type(checks[2][2]) == "tabletable" then
					local isGood=false
					local tapscan = Enum.Reverse(TapNoteScore)
					local holdscan = Enum.Reverse(HoldNoteScore)
					local finaltable = {{},{}} 
					for i=1,2 do
						for k,v in pairs(checks[2][i]) do
							finaltable[i][v]=true
							if (not (i==2 and holdscan[v] or tapscan[v])) or v==nil then isGood=false break end
							isGood=true
						end
						if not isGood then break end
					end
					if isGood then
						JudgmentFilters[k]=finaltable
					end
				end
			end
		end
	end
	return true
end

function UpdateScoreKeepers(JMC)
	assert(JMC,"pass args, please")
	runplayer = ToEnumShortString(JMC.Player)
	if ReadiedScoreKeepers[runplayer] then
		if JudgmentFilters[runplayer] and (JudgmentFilters[runplayer][1][JMC.TapNoteScore] or JudgmentFilters[runplayer][2][JMC.HoldNoteScore]) then
			return true
		end
		local pss = STATSMAN:GetCurStageStats():GetPlayerStageStats(JMC.Player)
		local newscores = {coroutine.resume(ReadiedScoreKeepers[runplayer],JMC,pss,JMC.Player,"update")}
		if CoroutineIsGood(ReadiedScoreKeepers[runplayer]) then
			pss:SetScore(newscores[2])
			pss:SetCurMaxScore(newscores[3])
			return true
		else
			SCREENMAN:SystemMessage("scoring update error: "..newscores[2])
			ReadiedScoreKeepers[runplayer]=nil
			return false
		end
	end
end

function FinalizeScoreKeepers()
	for k,v in pairs(ReadiedScoreKeepers) do
		local pss = STATSMAN:GetCurStageStats():GetPlayerStageStats("PlayerNumber_"..k)
		local newscores = {coroutine.resume(v,nil,pss,"PlayerNumber_"..k,"finalize")}
		local status = coroutine.status(v)
		--kill the scorekeeper now, we don't need it again
		ReadiedScoreKeepers[k] = nil
		if status=="dead" then
			if newscores[1]==true then	
				pss:SetScore(newscores[2])
				pss:SetCurMaxScore(newscores[3])
			else
				SCREENMAN:SystemMessage("scoring finalize error: "..newscores[2])
				return false
			end
		else
			return false
		end
	end
	return true
end

-- (c) 2012 Jack Walstrom and the spinal shark collective
-- All rights reserved.
--
-- Permission is hereby granted, free of charge, to any person obtaining a
-- copy of this software and associated documentation files (the
-- "Software"), to deal in the Software without restriction, including
-- without limitation the rights to use, copy, modify, merge, publish,
-- distribute, and/or sell copies of the Software, and to permit persons to
-- whom the Software is furnished to do so, provided that the above
-- copyright notice(s) and this permission notice appear in all copies of
-- the Software and that both the above copyright notice(s) and this
-- permission notice appear in supporting documentation.
--
-- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
-- OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
-- MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
-- THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
-- INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
-- OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
-- OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
-- OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
-- PERFORMANCE OF THIS SOFTWARE.