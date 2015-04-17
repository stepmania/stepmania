--[[
ThemePrefsRows: you give it the choices, values, and params, and it'll
generate the rest; quirky behavior to be outlined below. Documentation
will be provided once this system is stabilized.

v0.5.2: Dec. 28, 2010. Throw an error for default/value type mismatches.
v0.5.1: Dec. 27, 2010. Fix Choices not necessarily being strings.
v0.5.0: Dec. 15, 2010. Initial version. Not very well tested.

vyhd wrote this for sm-ssc
--]]

-- unless overridden, these parameters will be used for the OptionsRow
local DefaultParams =
{
	LayoutType = "ShowAllInRow",
	SelectType = "SelectOne",
	OneChoiceForAllPlayers = true,
	ExportOnChange = false,
	EnabledForPlayers = nil,
	ReloadRowMessages = nil,

	-- takes a function(self, list, pn);
	-- if not used, we use the default
	LoadSelections = nil,
	SaveSelections = nil,
}

-- local alias to simplify error reporting
local function GetString( name )
	return THEME:GetString( "ThemePrefsRows", name )
end

local function DefaultLoad( pref, default, choices, values )
	return function(self, list, pn)
		local val = ThemePrefs.Get( pref )

		-- if our current value is here, set focus to that
		for i=1, #choices do
			if values[i] == val then list[i] = true return end
		end

		-- try the default value
		for i=1, #choices do
			if values[i] == default then list[i] = true return end
		end

		-- set to the first value and output a warning
		Warn( GetString("NoDefaultInValues"):format(pref) )
		list[1] = true
	end
end

local function DefaultSave( pref, choices, values )
	local msg = "ThemePrefChanged"
	local params = { Name = pref }

	return function(self, list, pn)
		for i=1, #choices do
			if list[i] then
				ThemePrefs.Set( pref, values[i] )
				MESSAGEMAN:Broadcast( msg, params )
				break
			end
		end
	end
end

-- This function checks for mismatches between the default value and the
-- values table passed to the ThemePrefRow, e.g. it will return false if
-- you have a boolean default and an integer value. I'm somewhat stricter
-- about types than Lua is because I don't like the unpredictability and
-- complexity of coercing values transparently in a bunch of places.
local function TypesMatch( Values, Default )
	local DefaultType = type(Default)

	for i, value in ipairs(Values) do
		local ValueType = type(value)
		if ValueType ~= DefaultType then
			Warn( GetString("TypeMismatch"):format(DefaultType, i, ValueType) )
			return false
		end
	end

	return true
end

local function CreateThemePrefRow( pref, tbl )
	-- can't make an option handler without options
	if not tbl.Choices then return nil end

	local Choices = tbl.Choices
	local Default = tbl.Default
	local Values = tbl.Values and tbl.Values or Choices
	local Params = tbl.Params and tbl.Params or { }

	-- if the choices aren't strings, make them strings now
	for i, str in ipairs(Choices) do
		Choices[i] = tostring( Choices[i] )
	end

	-- check to see that Values and Choices have the same length
	if #Choices ~= #Values then
		Warn( GetString("ChoicesSizeMismatch") )
		return nil
	end

	-- check to see that everything in Values matches the type of Default
	if not TypesMatch( Values, Default ) then return nil end

	-- set the name and choices here; we'll do the rest below
	local Handler = { Name = pref, Choices = Choices }

	-- add all the keys in DefaultParams, get the value from
	-- Params if it exists and DefaultParams otherwise
	-- (note that we explicitly check for nil, due to bools.)
	for k, _ in pairs(DefaultParams) do
		Handler[k] = Params[k] ~= nil and Params[k] or DefaultParams[k]
	end

	-- if we don't have LoadSelections and SaveSelections, make them
	if not Handler.LoadSelections then
		Handler.LoadSelections = DefaultLoad( pref, Default, Choices, Values )
	end

	if not Handler.SaveSelections then
		Handler.SaveSelections = DefaultSave( pref, Choices, Values )
	end

	return Handler
end

-- All OptionsRows for preferences are stuck in this table, accessible
-- through GetRow('name') in the namespace or ThemePrefRow('name') in
-- the global namespace. (I like to keep pollution to a minimum.)
local Rows = { }

ThemePrefsRows =
{
	IsInitted = false,

	Init = function( prefs )
		for pref, tbl in pairs(prefs) do
			Rows[pref] = CreateThemePrefRow( pref, tbl )
		end
	end,

	GetRow = function( pref )
		Trace( ("GetRow(%s), type %s"):format(pref, type(Rows[pref])) )
		return Rows[pref]
	end,

}

-- Global namespace alias
ThemePrefRow = ThemePrefsRows.GetRow

-- UGLY: declare this here, even though it's in the previous namespace,
-- so we can have one call to initialize both systems (ThemePrefsRow is
-- declared after ThemePrefs, so it can't actually be in that file...)

ThemePrefs.InitAll = function( prefs )
	Trace( "ThemePrefs.InitAll( prefs )" )

	ThemePrefs.Init( prefs, true )
	ThemePrefsRows.Init( prefs )
end
