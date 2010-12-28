--[[
ThemePrefsRows: you give it the choices, values, and params, and it'll
generate the rest; quirky behavior to be outlined below. Documentation
will be provided once this system is stabilized.

v0.6: Dec. 27, 2010. Fix Choices not necessarily being strings.
v0.5: Dec. 15, 2010. Initial version. Not very well tested.
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

local function DefaultLoadSelections( pref, default, choices, values )
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

		-- set to the first value and throw a warning
		Warn( ("LoadSelections: preference \"%s\"'s default not in Values"):format(pref) )
		list[1] = true
	end
end

local function DefaultSaveSelections( pref, choices, values )
	local msg = "ThemePrefChanged"
	local params = { Name = pref }

	return function(self, list, pn)
		for i=1, #choices do
			if list[i] then ThemePrefs.Set( pref, values[i] ) break end
			MESSAGEMAN:Broadcast( msg, params )
		end
	end
end


local function CreateThemePrefRow( pref, tbl )
	Trace( "CreateThemePrefRow( " .. pref .. " )" )

	-- can't make an option handler without options
	if not tbl.Choices then return nil end

	local Choices = tbl.Choices
	local Values = tbl.Values and tbl.Values or Choices
	local Params = tbl.Params and tbl.Params or { }

	-- if the choices aren't strings, make them strings now
	for i, str in ipairs(Choices) do
		Choices[i] = tostring( Chances[i] )
	end

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
		Handler.LoadSelections = DefaultLoadSelections( pref, tbl.Default, Choices, Values )
	end

	if not Handler.SaveSelections then
		Handler.SaveSelections = DefaultSaveSelections( pref, Choices, Values )
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

	-- HACK: for now, don't worry about extraneous file I/O
	ThemePrefs.Init( prefs, true )
	ThemePrefsRows.Init( prefs )
end
