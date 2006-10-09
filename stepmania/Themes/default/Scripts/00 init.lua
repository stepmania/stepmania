-- This file is always executed first.

-- Override Lua's upper and lower functions with our own, which is always UTF-8.
string.upper = Uppercase
string.lower = Lowercase
Uppercase = nil -- don't use directly
Lowercase = nil -- don't use directly

PLAYER_1 = "PlayerNumber_P1"
PLAYER_2 = "PlayerNumber_P2"
NUM_PLAYERS = #PlayerNumber

function string:find_last( text )
	local LastPos = 0;
	while true do
		local p = string.find( self, text, LastPos+1, true );
		if not p then return LastPos end
		LastPos = p;
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

