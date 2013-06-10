local g_metrics_group = nil;
local g_element = nil;

function LoadFallbackB()
	-- Load the fallback BGA for the element that is currently being loaded.
	-- The fallback metrics group and element name will come either from LuaThreadVars
	-- (loading from C++) or from the Lua globals above (loading from Lua).
	
	--Warn( "g_element " .. (g_element or "") );
	--Warn( "MatchingElement " .. (Var 'MatchingElement' or "") );
	--Warn( "g_metrics_group " .. (g_metrics_group or "") );
	--Warn( "MatchingMetricsGroup " .. (Var 'MatchingMetricsGroup' or "") );

	local metrics_group = g_metrics_group or Var 'MatchingMetricsGroup';
	local element = g_element or Var 'MatchingElement';
	local fallback = THEME:GetMetric(metrics_group,'Fallback');
	
	local old_metrics_group = g_metrics_group;
	local old_element = g_element;
	
	local path;
	path, g_metrics_group, g_element = THEME:GetPathInfoB(fallback,element);
	--Trace('path ' .. path );
	local t = LoadActor( path );
	
	g_metrics_group = old_metrics_group;
	g_element = old_element;
	
	return t;
end

function FormatNumSongsPlayed( num )
	local s = num
	if s == 1 then 
		s = s .. ' song played'
	else 
		s = s .. ' songs played'
	end
	return s
end

function JudgmentTransformCommand( self, params )
	local x = 0
	local y = -30
	if params.bReverse then y = y * -1 end
	-- This makes no sense and wasn't even being used due to misspelling.
	-- if bCentered then y = y * 2 end
	self:x( x )
	self:y( y )
end

function JudgmentTransformSharedCommand( self, params )
	local x = -120
	local y = -30
	if params.bReverse then y = 30 end
	if params.Player == PLAYER_1 then x = 120 end
	self:x( x )
	self:y( y )
end

function ComboTransformCommand( self, params )
	local x = 0
	local y = 30
	if params.bReverse then y = y * -1 end

	if params.bCentered then
		if params.bReverse then
			y = y - 30
		else
			y = y + 40
		end
	end
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
	local sClass = Var "LoadingScreen";
	return THEME:GetMetric( sClass, sName )
end

function ScreenString( sName )
	local sClass = Var "LoadingScreen";
        return THEME:GetString( sClass, sName )
end

function TextBannerAfterSet(self,param) 
	local Title=self:GetChild("Title"); 
	local Subtitle=self:GetChild("Subtitle"); 
	--local Artist=self:GetChild("Artist"); 
	if Subtitle:GetText() == "" then 
		(cmd(maxwidth,208;y,0;zoom,1.0;))(Title);
		(cmd(visible,false))(Subtitle);
		--(cmd(zoom,0.66;maxwidth,300;y,7))(Artist);
	else
		-- subtitle below
		(cmd(zoom,1;y,-6;zoom,0.9;))(Title);
		(cmd(visible,true;zoom,0.6;y,7))(Subtitle); 
		--(cmd(zoom,0.66;maxwidth,300;y,9))(Artist); 
	end
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

