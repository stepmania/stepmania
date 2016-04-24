-- this file is outdated now, I think.
-- colors for each theme should live somewhere else because I'm nutty like that.

themeColors = {
	Player = {
		P1 = HSV(38,0.8,1),
		P2 = HSV(195,0.8,1),
        P3 = HSV(195,0.8,1),
        P4 = HSV(195,0.8,1)
	},
	Difficulty = {
		Beginner	= color("#ff32f8"),			-- light cyan
		Easy		= color("#2cff00"),			-- green
		Medium		= color("#fee600"),			-- yellow
		Hard		= color("#ff2f39"),			-- red
		Challenge	= color("#1cd8ff"),			-- light blue
		Edit		= color("0.8,0.8,0.8,1"),	-- gray
		Couple		= color("#ed0972"),			-- hot pink
		Routine		= color("#ff9a00"),			-- orange

		Difficulty_Beginner	= color("0.0,0.9,1.0,1"),		-- purple
		Difficulty_Easy		= color("0.9,0.9,0.0,1"),		-- green
		Difficulty_Medium	= color("1.0,0.1,0.1,1"),		-- yellow
		Difficulty_Hard		= color("0.2,1.0,0.2,1"),		-- red
		Difficulty_Challenge	= color("0.2,0.6,1.0,1"),	-- light blue
		Difficulty_Edit 	= color("0.8,0.8,0.8,1"),	-- gray
		Difficulty_Couple	= color("#ed0972"),			-- hot pink
		Difficulty_Routine	= color("#ff9a00"),			-- orange
	},
}

-- temporary:
function StageToColor( stage )
	return color("#FFFFFF")
end

function CustomDifficultyToColor(diff)
	return themeColors.Difficulty[diff] or color("1,0,0,1")
end;

-- judgment colors
local tnsColors = {
	TapNoteScore_W1 = HSV(48,0.2,1),
	TapNoteScore_W2 = HSV(48,0.8,0.95),
	TapNoteScore_W3 = HSV(160,0.9,0.8),
	TapNoteScore_W4 = HSV(200,0.9,1),
	TapNoteScore_W5 = HSV(320,0.9,1),
	TapNoteScore_Miss = HSV(0,0.8,0.8)
};
function TapNoteScoreToColor(tns) return tnsColors[tns] end
