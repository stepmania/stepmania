function PlayerColor( pn )
	if pn == PLAYER_1 then return color("#baa200") end	-- orange
	if pn == PLAYER_2 then return color("#83b767") end	-- sea green
	return color("1,1,1,1")
end

function PlayerScoreColor( pn )
	if pn == PLAYER_1 then return color("#ff6d00") end	-- orange
	if pn == PLAYER_2 then return color("#00ff7e") end	-- sea green
	return color("1,1,1,1")
end

-- let driving themes override just this table
GameCustomDifficultyColors = {
	Beginner	= color("#ff32f8"),
	Easy		= color("#2cff00"),
	Medium		= color("#fee600"),
	Hard		= color("#ff2f39"),
	Challenge	= color("#1cd8ff"),
};

CommonCustomDifficultyColors = {
	Edit		= color("0.8,0.8,0.8,1"),	-- gray
	Couple		= color("#ff9a00"),	-- orange
	Routine		= color("#ff9a00"),	-- orange
};

CourseDifficultyColors = {
	Difficulty_Beginner	= color("0.0,0.9,1.0,1"),	-- light blue
	Difficulty_Easy		= color("0.9,0.9,0.0,1"),	-- yellow
	Difficulty_Medium	= color("1.0,0.1,0.1,1"),	-- light red
	Difficulty_Hard		= color("0.2,1.0,0.2,1"),	-- light green
	Difficulty_Challenge	= color("0.2,0.6,1.0,1"),	-- blue
};

function CustomDifficultyToColor( sCustomDifficulty ) 
	local c = GameCustomDifficultyColors[sCustomDifficulty]
	if c then return c end
	c = CommonCustomDifficultyColors[sCustomDifficulty]
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

function StepsOrTrailToColor(StepsOrTrail)
	CustomDifficultyToColor( StepsOrTrailToCustomDifficulty(stepsOrTrail) );
end


StageColors = {
	Stage_1st	= color("#00ffc7"),
	Stage_2nd	= color("#58ff00"),
	Stage_3rd	= color("#f400ff"),
	Stage_4th	= color("#00ffda"),
	Stage_5th	= color("#ed00ff"),
	Stage_6th	= color("#73ff00"),
	Stage_Next	= color("#73ff00"),
	Stage_Final	= color("#ff0707"),
	Stage_Extra1	= color("#fafa00"),
	Stage_Extra2	= color("#ff0707"),
	Stage_Nonstop	= color("#FFFFFF"),
	Stage_Oni	= color("#FFFFFF"),
	Stage_Endless	= color("#FFFFFF"),
	Stage_Event	= color("#FFFFFF"),
	Stage_Demo	= color("#FFFFFF"),
};

function StageToColor( stage )
	local c = StageColors[stage];
	if c then return c end
	return color("#000000");
end

function StageToStrokeColor( stage )
	local c = StageToColor(stage);
	return { c[1]/2, c[2]/2, c[3]/2, c[4] };
end


JudgmentLineColors = {
	JudgmentLine_W1		= color("#00ffc7"),
	JudgmentLine_W2		= color("#f6ff00"),
	JudgmentLine_W3		= color("#00ff78"),
	JudgmentLine_W4		= color("#34bfff"),
	JudgmentLine_W5		= color("#e44dff"),
	JudgmentLine_Held	= color("#FFFFFF"),
	JudgmentLine_Miss	= color("#ff3c3c"),
	JudgmentLine_MaxCombo	= color("#ffc600"),
};

function JudgmentLineToColor( i )
	local c = JudgmentLineColors[i];
	if c then return c end
	return color("#000000");
end

function JudgmentLineToStrokeColor( i )
	local c = JudgmentLineToColor(i);
	return { c[1]/2, c[2]/2, c[3]/2, c[4] };
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

