--[[
ThemePrefs: handles the underlying structure for ThemePrefs, so any themes
built off of this can simply declare their prefs and default values, and
access them through this system.

v0.7.2: Jan. 10, 2012. Added ThemePrefs.ForceSave function [freem]
v0.7.1: Dec. 28, 2010. Added language support.
v0.7.0: Dec. 15, 2010. Initial version.

vyhd wrote this for sm-ssc. <3 you guys
--]]

-- local function to handle themed error strings
-- (and to ensure we're getting all of them from the same section)
local function GetString( name )
	return THEME:GetString( "ThemePrefs", name )
end

function PrintTable( tbl )
	Trace( "Printing table" )
	for k,v in pairs(tbl) do
		Trace( ("[%s] -> %s"):format(tostring(k),tostring(v)) )
	end
end

local ThemePrefsPath = "Save/ThemePrefs.ini";
local FallbackTheme = "_fallback";

-- This will be set on load.
local PrefsTable = nil;

-- Gets the name of the current theme using themeInfo
-- if available and the ThemeManager name otherwise.
local function GetThemeName()
	return themeInfo and themeInfo.Name or THEME:GetThemeDisplayName()
end

-- Given a preference name, returns the table it's in. Checks the current
-- theme first, then _fallback, then all other sections, in that order.
local function ResolveTable( pref )
	-- check the section for this theme
	local name = GetThemeName()
	local val = PrefsTable[name][pref]

	if val ~= nil then
		--Trace( ("ResolveTable(%s): found in %s"):format(pref,name) )
		return PrefsTable[name]
	end

	-- not in the current theme; check the fallback if it exists
	if PrefsTable[FallbackTheme] then
		val = PrefsTable[FallbackTheme][pref]
		if val ~= nil then
			--Trace( ("ResolveTable(%s): found in fallback"):format(pref) )
			return PrefsTable[FallbackTheme]
		end
	end

	-- not there either. check every section.
	-- XXX: we should do this less redundantly.
	for section, _ in pairs(PrefsTable) do
		val = PrefsTable[section][pref]
		if val ~= nil then
			--Trace( ("ResolveTable(%s): found in section %s"):format(pref,section) )
			return PrefsTable[section] end
	end

	-- not found at all
	Trace( ("ResolveTable(%s): pref not found"):format(pref) )
	return nil
end

ThemePrefs =
{
	NeedsSaved = false,

	-- Loads preferences from Save/ThemePrefs.ini, then adds theme
	-- preferences (and default values if applicable) to PrefsTable.
	-- Only read from disk once, when _fallback calls this; we just
	-- need the base set once to add prefs onto.
	Init = function( prefs, bLoadFromDisk )

		-- If we don't have IniFile, we can't read/write from/to disk
		if not IniFile then Warn( GetString("IniFileMissing") ) end

--		Trace( ("ThemePrefs.Init(prefs, %s)"):format(tostring(bLoadFromDisk)) )
		if bLoadFromDisk then
			Trace( "ThemePrefs.Init: loading from disk" )
			if not ThemePrefs.Load() then return false end
		end

--		Trace( "ThemePrefs.Init: not loading from disk" )

		-- create the section if it doesn't exist
		local section = GetThemeName()
--		Trace( ("ThemePrefs.Init: Theme name is \"%s\""):format(section) )
		PrefsTable[section] = PrefsTable[section] and PrefsTable[section] or { }

		--Trace( "Using section " .. section )

		-- if the key doesn't exist, add it with our default value
		for k, tbl in pairs(prefs) do
			if PrefsTable[section][k] == nil then
				Trace( k .. " doesn't exist, creating" )
				PrefsTable[section][k] = tbl.Default
			end
		end

--		PrintTable( PrefsTable[section] )
	end,

	Load = function()
		if not IniFile then return false end
		PrefsTable = IniFile.ReadFile( ThemePrefsPath )
		return true
	end,

	Save = function()
--		Trace( "ThemePrefs.Save" )
		if IniFile and ThemePrefs.NeedsSaved then
			IniFile.WriteFile( ThemePrefsPath, PrefsTable )
			ThemePrefs.NeedsSaved = false
			return
		end
	end,

	-- for when you absolutely have to save, no matter what NeedsSaved says.
	ForceSave = function()
		if not IniFile then return false end
		ThemePrefs.NeedsSaved = false
		IniFile.WriteFile( ThemePrefsPath, PrefsTable )
	end,

	Get = function( name )
		--Trace( ("ThemePrefs.Get(%s)"):format(name) )
		local tbl = ResolveTable(name)
		if tbl then return tbl[name] end
		Warn( "Get: "..GetString("UnknownPreference"):format(name) )
		return nil
	end,

	Set = function( name, value )
		--Trace( ("ThemePrefs.Set(%s, %s)"):format(name, tostring(value)) )
		local tbl = ResolveTable(name)
		if tbl then
			ThemePrefs.NeedsSaved = true
			tbl[name] = value
			return
		end
		Warn( "Set: "..GetString("UnknownPreference"):format(name) )
	end,
};

-- global aliases
GetThemePref = ThemePrefs.Get
SetThemePref = ThemePrefs.Set


-- bring in SpecialScoring from default.

function InitUserPrefs()
	if GetUserPref("UserPrefScoringMode") == nil then
		SetUserPref("UserPrefScoringMode", 'DDR Extreme');
	end;
end;
