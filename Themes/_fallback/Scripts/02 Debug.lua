if not debug then
	-- stubs
	debug = {}
	debug.traceback = function() return "" end
	return
end

-- Override debug.traceback.
function debug.traceback(...)
	local thread = coroutine.running()
	local msg = ""
	local level = 1
	local args = {...}
	
	if type(args[1]) == "thread" then
		thread = args[1]
		table.remove(args)
	end
	if type(args[1]) == "string" then
		msg = args[1]
		table.remove(args)
	end
	if type(args[1]) == "number" then
		level = args[1]
		table.remove(args)
	end

	if thread == coroutine.running() then
		level = level + 1 -- skip this function
	end

	local stack = {}
	repeat
		local info = debug.getinfo(level, "Sln")
		table.insert( stack, info )
		level = level + 1
	until not info

	if #stack == 0 then
		return ""
	end

	-- The original caller is usually C; remove it.
	if( stack[#stack].what == "C" ) then
		table.remove( stack )
	end
	local function FormatFrame(level, frame)
		local sFrameInfo = ""
		sFrameInfo = sFrameInfo .. "#" .. level .. "  "
		if( frame.what == "tail" ) then
		--      sFrameInfo = sFrameInfo .. "(tail call)"
		--      return sFrameInfo
		elseif( frame.what == "main" ) then
			sFrameInfo = sFrameInfo .. "main in "
		elseif frame.name then
			sFrameInfo = sFrameInfo .. frame.name .. "() in "
		else
			sFrameInfo = sFrameInfo .. "... in "
		end
			sFrameInfo = sFrameInfo .. frame.short_src

		if frame.currentline ~= -1 then
			sFrameInfo = sFrameInfo .. ":" .. frame.currentline
		end

		return sFrameInfo
	end

	local FrameInfo = {}
	for level, frame in ipairs(stack) do
		FrameInfo[level] = FormatFrame(level, frame)
	end
	return table.concat( FrameInfo, "\n" )
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
