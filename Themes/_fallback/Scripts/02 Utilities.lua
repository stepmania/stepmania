-- Find a key in tab with the given value.
function FindValue(tab, value)
	for key, name in tab do
		if value == name then
			return key
		end
	end

	return nil
end

-- Return the index of a true value in list.
function FindSelection(list)
	for index, on in list do
		if on then
			return index
		end
	end
	return nil
end

-- Look up each value in a table, returning a table with the resulting strings.
function TableStringLookup(t, group)
	local ret = { }
	for key, val in t do
		Trace(val)
		ret[key] = THEME:GetString(group,val)
	end
	return ret
end

function split(delimiter, text)
	local list = {}
	local pos = 1
	while 1 do
		local first,last = string.find(text, delimiter, pos)
		if first then
			table.insert(list, string.sub(text, pos, first-1))
			pos = last+1
		else
			table.insert(list, string.sub(text, pos))
			break
		end
	end
	return list
end

function join(delimiter, list)
	local ret = list[1]
	for i = 2,table.getn(list) do
		ret = ret .. delimiter .. list[i]
	end
	return ret or ""
end

function wrap(val,n)
	local x = val
	Trace( "wrap "..x.." "..n )
	if x<0 then 
		x = x + (math.ceil(-x/n)+1)*n
	end
	Trace( "adjusted "..x )
	local ret = math.mod(x,n)
	Trace( "ret "..ret )
	return ret
end

function fapproach(val, other_val, to_move)
	if val == other_val then
		return val -- already done!
	end

	local delta = other_val - val
	local sign = delta / math.abs(delta)
	local toMove = sign*to_move

	if math.abs(toMove) > math.abs(delta) then
		toMove = delta	-- snap
	end

	val = val + toMove
	return val
end

function tableshuffle(t)
	local ret = {}
	for i=1,table.getn(t) do
		table.insert(ret, math.random(i), t[i])
	end
	return ret
end
table.shuffle = tableshuffle

function tableslice(t, num)
	local ret = {}
	for i=1,table.getn(t) do
		table.insert(ret, i, t[i])
	end
	return ret
end
table.slice = tableslice

-- add together the contents of a table
function table.sum(t)
	local sum = 0
	for i=1,#t do
		sum = sum + t[i]
	end
	return sum
end

-- average of all table values
function table.average(t)
	return table.sum(t) / #t
end

-- furthest value from the average of a given table
function table.deviation(t)
	local offset = math.abs(table.average(t))
	for i=1,#t do
		offset = math.max(math.abs(t[i]), offset)
	end
	return offset
end
-- See if this exists
function table.search(t, sFind)
	for _, v in pairs(t) do
		if v == sFind then
			return true
		end
	end
	return false
end
-- Retreive the entry that has this
function table.find(t, sFind)
	for _, v in pairs(t) do
		if v == sFind then
			return i
		end
	end
	return nil
end

function round(val, decimal)
	if decimal then
		return math.floor((val * 10^decimal) + 0.5) / (10^decimal)
	else
		return math.floor(val+0.5)
	end
end

function GetRandomSongBackground()
	for i=0,50 do
		local song = SONGMAN:GetRandomSong()
		if song then
			local path = song:GetBackgroundPath()
			if path then
				return path
			end
		end
	end
	return THEME:GetPathG("", "_blank")
end

function GetSongBackground()
	local song = GAMESTATE:GetCurrentSong()
	if song then
		local path = song:GetBackgroundPath()
		if path then
			return path
		end
	end
	return THEME:GetPathG("Common","fallback background")
end

function StepsOrTrailToCustomDifficulty(stepsOrTrail)
	if lua.CheckType("Steps", stepsOrTrail) then
		return StepsToCustomDifficulty(stepsOrTrail)
	end
	if lua.CheckType("Trail", stepsOrTrail) then
		return TrailToCustomDifficulty(stepsOrTrail)
	end
end

-- (c) 2005 Glenn Maynard, Chris Danford
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

