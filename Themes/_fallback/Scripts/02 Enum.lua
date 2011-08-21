function Enum:Compare( e1, e2 )
	local Reverse = self:Reverse()
	local Value1 = Reverse[e1]
	local Value2 = Reverse[e2]

	assert( Value1, tostring(e1) .. " is not an enum of type " .. self:GetName() )
	assert( Value2, tostring(e2) .. " is not an enum of type " .. self:GetName() )

	-- Nil enums correspond to "invalid". These compare greater than any valid
	-- enum value, to line up with the equivalent C++ code.

	-- should this be changed to math.huge()? -shake
	if not e1 then
		Value1 = 99999999
	end
	if not e2 then
		Value2 = 99999999
	end
	return Value1 - Value2
end

function ToEnumShortString( e )
	local pos = string.find( e, '_' )
	assert( pos, "'" .. e .. "' is not an enum value" )
	return string.sub( e, pos+1 )
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
