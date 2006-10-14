function FormatNumSongsPlayed( num )
	local s = num
	if s == 1 then 
		s = s .. ' song played'
	else 
		s = s .. ' songs played'
	end
	return s
end

function JudgmentTransformCommand( self, pn, mp, iEnabledPlayerIndex, iNumEnabledPlayers, bUsingBothSides, bReverse, bCentered )
	local x = 0
	local y = -30
	if bReverse then y = y * -1 end
	-- This makes no sense and wasn't even being used due to misspelling.
	-- if bCentered then y = y * 2 end
	self:x( x )
	self:y( y )
end

function JudgmentTransformSharedCommand( self, pn, mp, iEnabledPlayerIndex, iNumEnabledPlayers, bUsingBothSides, bReverse, bCentered )
	local x = -120
	local y = -30
	if bReverse then y = 30 end
	if pn == 1 then x = 120 end
	self:x( x )
	self:y( y )
end

function GetEditModeSubScreens()
	return
		"ScreenMiniMenuEditHelp," ..
		"ScreenMiniMenuMainMenu," ..
		"ScreenMiniMenuAreaMenu," ..
		"ScreenMiniMenuStepsInformation," ..
		"ScreenMiniMenuSongInformation," ..
		"ScreenMiniMenuBackgroundChange," ..
		"ScreenMiniMenuInsertTapAttack," ..
		"ScreenMiniMenuInsertCourseAttack," ..
		"ScreenMiniMenuCourseDisplay," ..
		"ScreenEditOptions";
end

function GetCoursesToShowRanking()
	local CoursesToShowRanking = PREFSMAN:GetPreference("CoursesToShowRanking")
	if CoursesToShowRanking ~= "" then return CoursesToShowRanking end
	return "Courses/Samples/Tournamix 4 Sample.crs,Courses/Samples/StaminaTester.crs,Courses/Samples/PlayersBest1-4.crs"
end

-- Get a metric from the currently-loading screen.  This is only valid while loading
-- an actor, such as from File or InitCommand attributes; not from commands.
function ScreenMetric( sName )
	local sClass = LoadingScreen;
	return THEME:GetMetric( sClass, sName )
end

function ScreenString( sName )
	local sClass = LoadingScreen;
        return THEME:GetString( sClass, sName )
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

