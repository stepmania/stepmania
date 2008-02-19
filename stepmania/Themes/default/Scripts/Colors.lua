function PlayerColor( pn )
	if pn == PLAYER_1 then return color("0.4,1.0,0.8,1") end	-- sea green
	if pn == PLAYER_2 then return color("1.0,0.5,0.2,1") end	-- orange
	return color("1,1,1,1")
end

local DifficultyColors = {
	Difficulty_Beginner	= color("0.0,0.9,1.0,1"),	-- light blue
	Difficulty_Easy		= color("0.9,0.9,0.0,1"),	-- yellow
	Difficulty_Medium	= color("1.0,0.1,0.1,1"),	-- light red
	Difficulty_Hard		= color("0.2,1.0,0.2,1"),	-- light green
	Difficulty_Challenge	= color("0.2,0.6,1.0,1"),	-- blue
	Difficulty_Edit		= color("0.8,0.8,0.8,1"),	-- gray
}

function DifficultyToColor( dc ) 
	local c = DifficultyColors[dc]
	if c then return c end
	return color("#000000");
end



function StepsOrTrailToLocalizedString(StepsOrTrail)
	if not StepsOrTrail then
		return "";
	end
	if lua.CheckType("Trail", StepsOrTrail) then
		local s = THEME:GetString("DifficultyAndStepsType", "Course");
		return s;
	end

	local dc = StepsOrTrail:GetDifficulty();
	local st = StepsOrTrail.GetStepsType and StepsOrTrail:GetStepsType();
	return DifficultyAndStepsTypeToLocalizedString( dc, st );
end

function StepsOrTrailToIndex(StepsOrTrail)
	if not StepsOrTrail then return 1; end
	if lua.CheckType("Trail", StepsOrTrail) then
		return 8
	end
	local dc = StepsOrTrail:GetDifficulty();
	local st = StepsOrTrail.GetStepsType and StepsOrTrail:GetStepsType();
	return DifficultyAndStepsTypeToIndex( dc, st );
end;
function TrailColor()
	return color("#30f0c8");
end;

function StepsOrTrailToColor(StepsOrTrail)
	if not StepsOrTrail then return color("#000000"); end
	if lua.CheckType("Trail", StepsOrTrail) then return TrailColor() end

	local dc = StepsOrTrail:GetDifficulty();
	local st = StepsOrTrail.GetStepsType and StepsOrTrail:GetStepsType();
	return DifficultyAndStepsTypeToColor( dc, st );
end

function DifficultyAndStepsTypeToLocalizedString( difficulty, stepsType )
	if( not difficulty or not stepsType ) then
		return "";
	end
	return THEME:GetString( "DifficultyAndStepsType", difficulty .. "-" .. stepsType );
end

function DifficultyAndStepsTypeToIndex( difficulty, stepsType )
	for idx, entry in ipairs(DifficultyAndStepsTypeInfo) do
		if entry.difficulty == difficulty and entry.stepsType == stepsType then 
			return idx;
		end
	end

	if difficulty == "Difficulty_Edit" then
		return 7
	end;
	return nil;
end

function DifficultyAndStepsTypeToColor( difficulty, stepsType )
	return DifficultyToColor( difficulty );
end




-- (c) 2005 Chris Danford
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

