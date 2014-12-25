function AreStagePlayerModsForced()
	return GAMESTATE:IsAnExtraStage() or (GAMESTATE:GetPlayMode() == "PlayMode_Oni")
end

function AreStageSongModsForced()
	local pm = GAMESTATE:GetPlayMode()
	local bOni = pm == "PlayMode_Oni"
	local bBattle = pm == "PlayMode_Battle"
	local bRave = pm == "PlayMode_Rave"
	return GAMESTATE:IsAnExtraStage() or bOni or bBattle or bRave
end

function ScreenSelectMusic:setupmusicstagemods()
	Trace( "setupmusicstagemods" )
	local pm = GAMESTATE:GetPlayMode()

	if pm == "PlayMode_Battle" or pm == "PlayMode_Rave" then
		local so = GAMESTATE:GetDefaultSongOptions()
		GAMESTATE:SetSongOptions( "ModsLevel_Stage", so )
		MESSAGEMAN:Broadcast( "SongOptionsChanged" )
	elseif GAMESTATE:IsAnExtraStage() then
		if GAMESTATE:GetPreferredSongGroup() == "---Group All---" then
			local song = GAMESTATE:GetCurrentSong()
			GAMESTATE:SetPreferredSongGroup( song:GetGroupName() )
		end

		local bExtra2 = GAMESTATE:IsExtraStage2()
		local style = GAMESTATE:GetCurrentStyle()
		local song, steps = SONGMAN:GetExtraStageInfo( bExtra2, style )
		local po, so
		if bExtra2 then
			po = THEME:GetMetric("SongManager","OMESPlayerModifiers")
			so = THEME:GetMetric("SongManager","OMESStageModifiers")
		else
			po = THEME:GetMetric("SongManager","ExtraStagePlayerModifiers")
			so = THEME:GetMetric("SongManager","ExtraStageStageModifiers")
		end

		local difficulty = steps:GetDifficulty()
		local Reverse = PlayerNumber:Reverse()

		GAMESTATE:SetCurrentSong( song )
		GAMESTATE:SetPreferredSong( song )

		for pn in ivalues(GAMESTATE:GetHumanPlayers()) do
			GAMESTATE:SetCurrentSteps( pn, steps )
			GAMESTATE:GetPlayerState(pn):SetPlayerOptions( "ModsLevel_Stage", po )
			GAMESTATE:SetPreferredDifficulty( pn, difficulty )
			MESSAGEMAN:Broadcast( "PlayerOptionsChanged", {PlayerNumber = pn} )
		end

		GAMESTATE:SetSongOptions( "ModsLevel_Stage", so )
		MESSAGEMAN:Broadcast( "SongOptionsChanged" )
	end
	return self
end

function ScreenSelectMusic:setupcoursestagemods()
	local mode = GAMESTATE:GetPlayMode()

	if mode == "PlayMode_Oni" then
		local po = "clearall,default"
		-- Let SSMusic set battery.
		-- local so = "failimmediate,battery"
		local so = "failimmediate"
		local Reverse = PlayerNumber:Reverse()

		for pn in ivalues(GAMESTATE:GetHumanPlayers()) do
			GAMESTATE:GetPlayerState(pn):SetPlayerOptions( "ModsLevel_Stage", po )
			MESSAGEMAN:Broadcast( "PlayerOptionsChanged", {PlayerNumber = pn} )
		end

		GAMESTATE:SetSongOptions( "ModsLevel_Stage", so )
		MESSAGEMAN:Broadcast( "SongOptionsChanged" )
	end
	return self
end

-- (c) 2006-2007 Steve Checkoway
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

