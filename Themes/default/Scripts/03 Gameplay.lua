-- StepMania 5 default theme | script ring 03 | Gameplay.lua
-- someone thought it'd be a good idea to put theme preferences into the fallback
-- when they should've been in default instead. there is no emoticon for how i feel
-- at this moment right now -freem

-- for example, not every theme wants to worry about custom receptor positions.
local tNotePositions = {
	-- StepMania 3.9/4.0
	Normal = { -144, 144, },
	-- ITG
	Lower = { -125, 145, }
}

function GetTapPosition( sType )
	bCategory = (sType == 'Standard') and 1 or 2
	-- true: Normal
	-- false: Lower
	bPreference = ThemePrefs.Get("NotePosition") and "Normal" or "Lower"
	tNotePos = tNotePositions[bPreference]
	return tNotePos[bCategory]
end

-- combo under field is another thing that doesn't always need to be custom
function ComboUnderField()
	return ThemePrefs.Get("ComboUnderField")
end

-- The "boss song" threshold is controlled entirely by a theme preference.
function GetExtraColorThreshold()
	local ret = {
		["old"] = 10,
		["X"] = 15,
		["pump"] = 21,
	}
	return ret[ThemePrefs.Get("PreferredMeter")] or 10
end

-- These judgment-related things are also controlled by theme preferences,
-- but allow using the default gametype-based setting just in case.

-- Lowest judgment allowed to increment a combo.
function ComboContinue()
	local Continue = {
		dance = GAMESTATE:GetPlayMode() == "PlayMode_Oni" and "TapNoteScore_W2" or "TapNoteScore_W3",
		pump = "TapNoteScore_W3",
		beat = "TapNoteScore_W3",
		kb7 = "TapNoteScore_W3",
		para = "TapNoteScore_W4"
	}
	if ThemePrefs.Get("CustomComboContinue") ~= "default" then
		return ThemePrefs.Get("CustomComboContinue")
	else
		return Continue[GAMESTATE:GetCurrentGame():GetName()] or "TapNoteScore_W3"
	end
end

-- Lowest judgment allowed to maintain a combo; but not increment it.
function ComboMaintain()
	local Maintain = {
		dance = "TapNoteScore_W3",
		pump = "TapNoteScore_W4",
		beat = "TapNoteScore_W3",
		kb7 = "TapNoteScore_W3",
		para = "TapNoteScore_W4"
	}
	if ThemePrefs.Get("CustomComboMaintain") ~= "default" then
		return ThemePrefs.Get("CustomComboMaintain")
	else
		return Maintain[GAMESTATE:GetCurrentGame():GetName()] or "TapNoteScore_W3"
	end
end

-- The name's misleading; this is used for SelectPlayMode.
function ScreenSelectStylePositions(count)
	local poses= {}
	local choice_size = 192
	
	for i= 1, count do
		local start_x = _screen.cx + ( (choice_size / 1.5) * ( i - math.ceil(count/2) ) )
		-- The Y position depends on if the icon's index is even or odd.
		local start_y = i % 2 == 0 and _screen.cy / 0.8 or (_screen.cy / 0.8) - (choice_size / 1.5)
		poses[#poses+1] = {start_x, start_y}
	end
	
	return poses
end