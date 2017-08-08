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