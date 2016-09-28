-- This file is always executed first.

-- Override Lua's upper and lower functions with our own, which is always UTF-8.
if Uppercase then
	string.upper = Uppercase
	string.lower = Lowercase
	Uppercase = nil -- don't use directly
	Lowercase = nil -- don't use directly
end

Trace = lua.Trace
Warn = lua.Warn
print = Trace

-- Use MersenneTwister in place of math.random and math.randomseed.
if MersenneTwister then
	math.random = MersenneTwister.Random
	math.randomseed = MersenneTwister.Seed
end

PLAYER_1 = "PlayerNumber_P1"
PLAYER_2 = "PlayerNumber_P2"
NUM_PLAYERS = #PlayerNumber
OtherPlayer = { [PLAYER_1] = PLAYER_2, [PLAYER_2] = PLAYER_1 }

function string:find_last(text)
	local LastPos = 0
	while true do
		local p = string.find(self, text, LastPos+1, true)
		if not p then
			return LastPos
		end
		LastPos = p
	end
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
	return table.concat(list, delimiter)
end

function foreach_by_sorted_keys(tbl, keys, func)
	table.sort(keys)
	for _, key in ipairs(keys) do func(key, tbl[key]) end
end

function foreach_ordered( tbl, func )
	local string_keys= {}
	local number_keys= {}
	-- First person to to use this on a table that uses something else as keys
	-- gets to extend this function to cover more types.  And a beating. -Kyz
	for k,_ in pairs(tbl) do
		if type(k) == "string" then
			table.insert(string_keys, k)
		elseif type(k) == "number" then
			table.insert(number_keys, k)
		end
	end
	-- iterate in sorted order
	foreach_by_sorted_keys(tbl, number_keys, func)
	foreach_by_sorted_keys(tbl, string_keys, func)
end

function for_all_children(parent, func)
	local children= parent:GetChildren()
	for name, child in pairs(children) do
		if #child > 0 then
			for si, sc in ipairs(child) do
				func(sc)
			end
		else
			func(child)
		end
	end
end

-- (c) 2006 Glenn Maynard
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
