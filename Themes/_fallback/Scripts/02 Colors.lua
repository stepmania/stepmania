-- SSC Color Module and Library
local nilColor = color("0,0,0,0")
-- Original Color Module.
Color = {
-- Color Library
-- These colors are pure swatch colors and are here purely to be used
-- on demand without having to type color("stuff") or dig through 
-- a palette to get the color you want.
	__call(_color)	=	function(self, _color) return self[_color] end,
	Black		=	color("0,0,0,1"),
	White		=	color("1,1,1,1"),
	Red			=	color("#ed1c24"),
	Blue		=	color("#00aeef"),
	Green		=	color("#39b54a"),
	Yellow		=	color("#fff200"),
	Orange		=	color("#f7941d"),
	Purple		=	color("#92278f"),
	Outline		=	color("0,0,0,0.5"),
	Invisible	=	color("1,1,1,0"),
	Stealth		=	nilColor,
-- Color Functions
-- These functions alter colors in a certain way so that you can make
-- new ones without having to copy a color or find a new one.
--[[     Brightness(fInput)
    Hue(hInput)
    Saturation(hInput)
    Alpha(hInput)
    HSV(iHue,fSaturation,fValue or any other overload) --]]
	Alpha = function(cColor,fAlpha)
		local c = cColor;
		return { c[1],c[2],c[3],fAlpha };
	end
}
-- Remapped Color Module, since some themes are crazy
-- Colors = Color;

GameColor = {
	PlayerColors = {
		PLAYER_1	= color("#ef403d"),
		PLAYER_2	= color("#0089cf"),
	},
	Difficulty = {
		--[[ These are for 'Custom' Difficulty Ranks. It can be very  useful
		in some cases, especially to apply new colors for stuff you
		couldn't before. (huh? -aj) ]]
		Beginner	= color("#ff32f8"),			-- light cyan
		Easy		= color("#2cff00"),			-- green
		Medium		= color("#fee600"),			-- yellow
		Hard		= color("#ff2f39"),			-- red
		Challenge	= color("#1cd8ff"),			-- light blue
		Edit		= color("0.8,0.8,0.8,1"),	-- gray
		Couple		= color("#ed0972"),			-- hot pink
		Routine		= color("#ff9a00"),			-- orange
		--[[ These are for courses, so let's slap them here in case someone
		wanted to use Difficulty in Course and Step regions. ]]
		Difficulty_Beginner	= color("#ff32f8"),		-- purple
		Difficulty_Easy		= color("#2cff00"),		-- green
		Difficulty_Medium	= color("#fee600"),		-- yellow
		Difficulty_Hard		= color("#ff2f39"),		-- red
		Difficulty_Challenge	= color("#1cd8ff"),	-- light blue
		Difficulty_Edit 	= color("0.8,0.8,0.8,1"),		-- gray
		Difficulty_Couple	= color("#ed0972"),				-- hot pink
		Difficulty_Routine	= color("#ff9a00")				-- orange
	},
	Stage = {
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
		Stage_Demo	= color("#FFFFFF")
	},
	Judgment = {
		JudgmentLine_W1		= color("#bfeaff"),
		JudgmentLine_W2		= color("#fff568"),
		JudgmentLine_W3		= color("#a4ff00"),
		JudgmentLine_W4		= color("#34bfff"),
		JudgmentLine_W5		= color("#e44dff"),
		JudgmentLine_Held	= color("#FFFFFF"),
		JudgmentLine_Miss	= color("#ff3c3c"),
		JudgmentLine_MaxCombo	= color("#ffc600")
	},
};
GameColor.Difficulty["Crazy"] = GameColor.Difficulty["Hard"];
GameColor.Difficulty["Freestyle"] = GameColor.Difficulty["Easy"];
GameColor.Difficulty["Nightmare"] = GameColor.Difficulty["Challenge"];
GameColor.Difficulty["HalfDouble"] = GameColor.Difficulty["Medium"];

--[[ Fallbacks ]]
function Color( in_c )
	return __call(in_c)
end --]]

function BoostColor( cColor, fBoost )
	local c = cColor
	return { c[1]*fBoost, c[2]*fBoost, c[3]*fBoost, c[4] }
end

function ColorLightTone(c)
	return { c[1]+(c[1]/2), c[2]+(c[2]/2), c[3]+(c[3]/2), c[4] }
end

function ColorMidTone(c)
	return { c[1]/1.5, c[2]/1.5, c[3]/1.5, c[4] }
end

function ColorDarkTone(c)
	return { c[1]/2, c[2]/2, c[3]/2, c[4] }
end

function PlayerColor( pn )
	if pn == PLAYER_1 then
		return color("#ef403d") -- pink-red
	end
	if pn == PLAYER_2 then
		return color("#0089cf") -- sea-blue
	end
	return color("1,1,1,1")
end
function PlayerScoreColor( pn )
	if pn == PLAYER_1 then
		return color("#ef403d") -- pink-red
	end
	if pn == PLAYER_2 then
		return color("#0089cf") -- sea-blue
	end
	return color("1,1,1,1")
end

function CustomDifficultyToColor( sCustomDifficulty ) 
	return GameColor.Difficulty[sCustomDifficulty]
end

function CustomDifficultyToDarkColor( sCustomDifficulty ) 
	local c = GameColor.Difficulty[sCustomDifficulty]
	return { c[1]/2, c[2]/2, c[3]/2, c[4] }
end

function CustomDifficultyToLightColor( sCustomDifficulty ) 
	local c = GameColor.Difficulty[sCustomDifficulty]
	return { scale(c[1],0,1,0.5,1), scale(c[2],0,1,0.5,1), scale(c[3],0,1,0.5,1), c[4] }
end

function StepsOrTrailToColor(StepsOrTrail)
	return CustomDifficultyToColor( StepsOrTrailToCustomDifficulty(stepsOrTrail) )
end

function StageToColor( stage )
	local c = GameColor.Stage[stage]
	if c then
		return c
	end
	return color("#000000")
end

function StageToStrokeColor( stage )
	local c = GameColor.Stage[stage]
	return { c[1]/2, c[2]/2, c[3]/2, c[4] }
end

function JudgmentLineToColor( i )
	local c = GameColor.Judgment[i]
	if c then
		return c
	end
	return color("#000000")
end

function JudgmentLineToStrokeColor( i )
	local c = GameColor.Judgment[i]
	return { c[1]/2, c[2]/2, c[3]/2, c[4] }
end