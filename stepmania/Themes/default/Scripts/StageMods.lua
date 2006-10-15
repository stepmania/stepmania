function AreStageModsForced()
	local bExtraStage = GAMESTATE:IsAnExtraStage()
	local bOni = GAMESTATE:GetPlayMode() == "PlayMode_Oni"
	return bExtraStage or bOni
end

function ScreenSelectMusic:setupmusicstagemods()
	if not GAMESTATE:IsAnExtraStage() then return end
	if GAMESTATE:GetPreferredSongGroup() == "---Group All---" and
	   not PREFSMAN:GetPreference("PickExtraStage") then
		local song = GAMESTATE:GetCurrentSong()
		GAMESTATE:SetPreferredSongGroup( song:GetGroupName() )
	end

	local bExtra2 = GAMESTATE:IsExtraStage2()
	local style = GAMESTATE:GetCurrentStyle()
	local song, steps, po, so = SONGMAN:GetExtraStageInfo( bExtra2, style )
	local difficulty = steps:GetDifficulty()
	
	GAMESTATE:SetCurrentSong( song )
	GAMESTATE:SetPreferredSong( song )

	for _, pn in ipairs(GAMESTATE:GetHumanPlayers()) do
		GAMESTATE:SetCurrentSteps( pn, steps )
		GAMESTATE:GetPlayerState(pn):SetPlayerOptions( "ModsLevel_Stage", po )
		GAMESTATE:SetPreferredDifficulty( pn, difficulty )
		MESSAGEMAN:Broadcast( "PlayerOptionsChangedP" .. (pn+1) )
	end
	
	GAMESTATE:SetSongOptions( "ModsLevel_Stage", so )
	MESSAGEMAN:Broadcast( "SongOptionsChanged" )
end

function ScreenSelectMusic:setupcoursestagemods()
	local mode = GAMESTATE:GetPlayMode()
	
	if mode == "PlayMode_Oni" then
		local po = "clearall," .. NOTESKIN:GetGameBaseNoteSkinName()
		-- Let SSMusic set battery.
		-- local so = "failimmediate,battery"
		local so = "failimmediate"

		for dummy, pn in ipairs(GAMESTATE:GetHumanPlayers()) do
			GAMESTATE:GetPlayerState(pn):SetPlayerOptions( "ModsLevel_Stage", po )
			MESSAGEMAN:Broadcast( "PlayerOptionsChangedP" .. (pn+1) )
		end

		GAMESTATE:SetSongOptions( "ModsLevel_Stage", so )
		MESSAGEMAN:Broadcast( "SongOptionsChanged" )
	end
end

-- 
-- (c) 2006 Steve Checkoway
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

