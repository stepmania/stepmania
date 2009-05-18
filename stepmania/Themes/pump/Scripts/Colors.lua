
local CustomDifficultyColors = {
	Novice		= color("#ff32f8"),
	Normal		= color("#2cff00"),
	Hard		= color("#fee600"),
	Crazy		= color("#ff2f39"),
	HalfDouble	= color("#33ff33"),	-- greenish
	Freestyle	= color("#A020A0"),	-- purplish
	Nightmare	= color("#1cd8ff"),
	Edit		= color("0.8,0.8,0.8,1"),	-- gray
};

function CustomDifficultyToColor( sCustomDifficulty ) 
	local c = CustomDifficultyColors[sCustomDifficulty]
	if c then return c end
	return color("#000000");
end

function CustomDifficultyToDarkColor( sCustomDifficulty ) 
	local c = CustomDifficultyToColor(sCustomDifficulty);
	return { c[1]/2, c[2]/2, c[3]/2, c[4] };
end

function CustomDifficultyToLightColor( sCustomDifficulty ) 
	local c = CustomDifficultyToColor(sCustomDifficulty);
	return { scale(c[1],0,1,0.5,1), scale(c[2],0,1,0.5,1), scale(c[3],0,1,0.5,1), c[4] };
end

function CourseDifficutlyToColor( cd )
	local c = CourseDifficultyColors[cd]
	if c then return c end
	return color("#000000");
end




-- (c) 2009 Chris Danford, Jason Felds
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

