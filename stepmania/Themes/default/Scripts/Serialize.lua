-- Serialize the table "t".
function Serialize(t)
	local ret = ""
	local queue = { }
	local already_queued = { }

	-- Convert a value to an identifier.  If we encounter a table that we've never seen before,
	-- it's an anonymous table and we'll create a name for it; for example, in t = { [ {10} ] = 1 },
	-- "{10}" has no name.
	local next_id = 1
	local function convert_to_identifier( v, name )
		-- print("convert_to_identifier: " .. (name or "nil"))
		if type(v) == "string" then
			return string.format("%q", v)
		elseif type(v) == "nil" then
			return "nil"
		elseif type(v) == "boolean" then
			if v then return "true" end
			return "false"
		elseif type(v) == "number" then
			return string.format("%i", v)
		elseif type(v) == "table" then
			if already_queued[v] then
				return already_queued[v]
			end

			-- Create the table.  If we have no name, give it one; be sure to make it local.
			if not name then
				name = "tab" .. next_id
				next_id = next_id + 1
				ret = ret .. "local " .. name .. " = { }\n"
			else
				-- The name is probably something like "x[1][2][3]", so don't emit "local".
				ret = ret .. name .. " = { }\n"
			end

			for i, tab in pairs(v) do
				local to_fill = { ["name"] = name .. "[" .. convert_to_identifier(i) .. "]", with = tab }
				table.insert( queue, to_fill )
			end

			already_queued[v] = name
			return name
		else
			return '"UNSUPPORTED TYPE (' .. type(v) .. ')"', true
		end
	end

	local top_name = convert_to_identifier( t )
 
	while table.getn(queue) > 0 do
		local to_fill = table.remove( queue, 1 )
		local str = convert_to_identifier( to_fill.with, to_fill.name )
		-- Assign the result.  If to_fill.with is a non-anonymous table, we just created
		-- it ("ret[1] = { }"); don't redundantly write "ret[1] = ret[1]".
		if to_fill.name ~= str then
			ret = ret .. to_fill.name .. " = " .. str .. "\n"
		end
	end
	ret = ret .. "return " .. top_name
	return ret
end

-- Recursively deep-copy a table.
function DeepCopy(t, already_copied)
	already_copied = already_copied or { }

	if type(t) ~= "table" then
		return t
	end

	if already_copied[t] then
		return already_copied[t]
	end
	already_copied[t] = { }
	local ret = already_copied[t]

	table.foreach(t, function(a,b) ret[a] = DeepCopy(b, already_copied) end)
	return ret
end

-- (c) 2005 Glenn Maynard
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

