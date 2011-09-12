-- sm-ssc Default Theme Preferences Handler

-- Example usage of new system (not really implemented yet)
local Prefs =
{
	AutoSetStyle =
	{
		Default = false,
		Choices = { "ON", "OFF" },
		Values = { true, false }
	},
}

ThemePrefs.InitAll(Prefs)

function InitUserPrefs()
	local Prefs = {
		UserPrefGameplayShowStepsDisplay = true,
		UserPrefGameplayShowStepsDisplay = true,
		UserPrefGameplayShowScore = false,
		UserPrefScoringMode = 'DDR Extreme',
		UserPrefShowLotsaOptions = true,
		UserPrefAutoSetStyle = false,
		UserPrefLongFail = false,
		UserPrefNotePosition = true,
		UserPrefComboOnRolls = false,
		UserPrefProtimingP1 = false,
		UserPrefProtimingP2 = false,
		FlashyCombos = false,
		UserPrefComboUnderField = true,
		UserPrefFancyUIBG = true
	}
	for k, v in pairs(Prefs) do
		-- kind of xxx
		local GetPref = type(v) == "boolean" and GetUserPrefB or GetUserPref
		if GetPref(k) == nil then
			SetUserPref(k, v)
		end
	end

	-- screen filter
	setenv("ScreenFilterP1",0)
	setenv("ScreenFilterP2",0)
end

function GetProTiming(pn)
	local pname = ToEnumShortString(pn)
	if GetUserPref("ProTiming"..pname) then
		return GetUserPrefB("ProTiming"..pname)
	else
		SetUserPref("ProTiming"..pname,false)
		return false
	end
end

--[[ option rows ]]

-- screen filter
function OptionRowScreenFilter()
	local t = {
		Name="ScreenFilter",
		LayoutType = "ShowAllInRow",
		SelectType = "SelectOne",
		OneChoiceForAllPlayers = false,
		ExportOnChange = false,
		Choices = { 'Off', '0.1', '0.2', '0.3', '0.4', '0.5', '0.6', '0.7', '0.8', '0.9', '1.0', },
		LoadSelections = function(self, list, pn)
			local pName = ToEnumShortString(pn)
			local filterValue = getenv("ScreenFilter"..pName)
			if filterValue ~= nil then
				local val = scale(tonumber(filterValue),0,1,1,#list )
				list[val] = true
			else
				setenv("ScreenFilter"..pName,0)
				list[1] = true
			end
		end,
		SaveSelections = function(self, list, pn)
			local pName = ToEnumShortString(pn)
			local found = false
			for i=1,#list do
				if not found then
					if list[i] == true then
						local val = scale(i,1,#list,0,1)
						setenv("ScreenFilter"..pName,val)
						found = true
					end
				end
			end
		end,
	};
	setmetatable(t, t)
	return t
end

-- protiming
function OptionRowProTiming()
	local t = {
		Name = "ProTiming",
		LayoutType = "ShowAllInRow",
		SelectType = "SelectOne",
		OneChoiceForAllPlayers = false,
		ExportOnChange = false,
		Choices = { 'Off','On' },
		LoadSelections = function(self, list, pn)
			if GetUserPrefB("UserPrefProtiming" .. ToEnumShortString(pn)) then
				local bShow = GetUserPrefB("UserPrefProtiming" .. ToEnumShortString(pn))
				if bShow then
					list[2] = true
				else
					list[1] = true
				end
			else
				list[1] = true
			end
		end,
		SaveSelections = function(self, list, pn)
			local bSave = list[2] and true or false
			SetUserPref("UserPrefProtiming" .. ToEnumShortString(pn), bSave)
		end
	}
	setmetatable(t, t)
	return t
end

function UserPrefGameplayShowScore()
	local t = {
		Name = "UserPrefGameplayShowScore",
		LayoutType = "ShowAllInRow",
		SelectType = "SelectOne",
		OneChoiceForAllPlayers = true,
		ExportOnChange = false,
		Choices = { 'Off','On' },
		LoadSelections = function(self, list, pn)
			if ReadPrefFromFile("UserPrefGameplayShowScore") ~= nil then
				if GetUserPrefB("UserPrefGameplayShowScore") then
					list[2] = true
				else
					list[1] = true
				end
			else
				WritePrefToFile("UserPrefGameplayShowScore", false)
				list[1] = true
			end
		end,
		SaveSelections = function(self, list, pn)
			local val = list[2] and true or false
			WritePrefToFile("UserPrefGameplayShowScore", val)
			MESSAGEMAN:Broadcast("PreferenceSet", { Message == "Set Preference" })
			THEME:ReloadMetrics()
		end
	}
	setmetatable(t, t)
	return t
end

function UserPrefGameplayShowStepsDisplay()
	local t = {
		Name = "UserPrefGameplayShowStepsDisplay",
		LayoutType = "ShowAllInRow",
		SelectType = "SelectOne",
		OneChoiceForAllPlayers = true,
		ExportOnChange = false,
		Choices = { 'Off','On' },
		LoadSelections = function(self, list, pn)
			if ReadPrefFromFile("UserPrefGameplayShowStepsDisplay") ~= nil then
				if GetUserPrefB("UserPrefGameplayShowStepsDisplay") then
					list[2] = true
				else
					list[1] = true
				end
			else
				WritePrefToFile("UserPrefGameplayShowStepsDisplay", false)
				list[1] = true
			end
		end,
		SaveSelections = function(self, list, pn)
			local val = list[2] and true or false
			WritePrefToFile("UserPrefGameplayShowStepsDisplay", val)
			MESSAGEMAN:Broadcast("PreferenceSet", { Message == "Set Preference" })
			THEME:ReloadMetrics()
		end
	}
	setmetatable(t, t)
	return t
end

function UserPrefShowLotsaOptions()
	local t = {
		Name = "UserPrefShowLotsaOptions",
		LayoutType = "ShowAllInRow",
		SelectType = "SelectOne",
		OneChoiceForAllPlayers = true,
		ExportOnChange = false,
		Choices = { 'Many','Few' },
		LoadSelections = function(self, list, pn)
			if ReadPrefFromFile("UserPrefShowLotsaOptions") ~= nil then
				if GetUserPrefB("UserPrefShowLotsaOptions") then
					list[1] = true
				else
					list[2] = true
				end
			else
				WritePrefToFile("UserPrefShowLotsaOptions", false)
				list[2] = true
			end
		end,
		SaveSelections = function(self, list, pn)
			local val = list[1] and true or false
			WritePrefToFile("UserPrefShowLotsaOptions", val)
			MESSAGEMAN:Broadcast("PreferenceSet", { Message == "Set Preference" })
			THEME:ReloadMetrics()
		end
	}
	setmetatable(t, t);
	return t
end

function GetDefaultOptionLines()
	local LineSets = { -- none of these include characters yet.
		"1,8,14,2,3A,3B,4,5,6,R,7,9,10,11,12,13,16,SF,17", -- All
		"1,8,14,2,7,13,16,SF,17", -- DDR Essentials ( no turns, fx )
	};
	local function IsExtra()
		if GAMESTATE:IsExtraStage() or GAMESTATE:IsExtraStage2() then
			return true
		else
			return false
		end
	end
	
	local function CheckCharacters(mods)
		if CHARMAN:GetCharacterCount() > 0 then
			return mods .. ",18" --TODO: Better line name.
		end
		return mods
	end
	
	modLines = LineSets[2]
	
	if not IsExtra() then
		modLines = GetUserPrefB("UserPrefShowLotsaOptions")
			and LineSets[1] or LineSets[2]
	end
	
	return CheckCharacters(modLines)
end

function UserPrefAutoSetStyle()
	local t = {
		Name = "UserPrefAutoSetStyle",
		LayoutType = "ShowAllInRow",
		SelectType = "SelectOne",
		OneChoiceForAllPlayers = true,
		ExportOnChange = false,
		Choices = { 'Off','On' },
		LoadSelections = function(self, list, pn)
			if ReadPrefFromFile("UserPrefAutoSetStyle") ~= nil then
				if GetUserPrefB("UserPrefAutoSetStyle") then
					list[2] = true
				else
					list[1] = true
				end
			else
				WritePrefToFile("UserPrefAutoSetStyle", false)
				list[1] = true
			end
		end,
		SaveSelections = function(self, list, pn)
			local val = list[2] and true or false
			WritePrefToFile("UserPrefAutoSetStyle", val)
			MESSAGEMAN:Broadcast("PreferenceSet", { Message == "Set Preference" } )
			THEME:ReloadMetrics()
		end
	}
	setmetatable(t, t)
	return t
end

function UserPrefNotePosition()
	local t = {
		Name = "UserPrefNotePosition",
		LayoutType = "ShowAllInRow",
		SelectType = "SelectOne",
		OneChoiceForAllPlayers = true,
		ExportOnChange = false,
		Choices = { 'Normal','Lower' },
		LoadSelections = function(self, list, pn)
			if ReadPrefFromFile("UserPrefNotePosition") ~= nil then
				if GetUserPrefB("UserPrefNotePosition") then
					list[1] = true
				else
					list[2] = true
				end
			else
				WritePrefToFile("UserPrefNotePosition", false)
				list[1] = true
			end
		end,
		SaveSelections = function(self, list, pn)
			local val = list[1] and true or false
			WritePrefToFile("UserPrefNotePosition", val)
			MESSAGEMAN:Broadcast("PreferenceSet", { Message == "Set Preference" })
			THEME:ReloadMetrics()
		end
	}
	setmetatable(t, t)
	return t
end

function UserPrefLongFail()
	local t = {
		Name = "UserPrefLongFail",
		LayoutType = "ShowAllInRow",
		SelectType = "SelectOne",
		OneChoiceForAllPlayers = true,
		ExportOnChange = false,
		Choices = { 'Short','Long' },
		LoadSelections = function(self, list, pn)
			if ReadPrefFromFile("UserPrefLongFail") ~= nil then
				if GetUserPrefB("UserPrefLongFail") then
					list[2] = true
				else
					list[1] = true
				end
			else
				WritePrefToFile("UserPrefLongFail", false)
				list[1] = true
			end
		end,
		SaveSelections = function(self, list, pn)
			local val = list[2] and true or false
			WritePrefToFile("UserPrefLongFail", val)
			MESSAGEMAN:Broadcast("PreferenceSet", { Message == "Set Preference" } )
			THEME:ReloadMetrics()
		end
	}
	setmetatable(t, t)
	return t
end

function UserPrefComboOnRolls()
	local t = {
		Name = "UserPrefComboOnRolls",
		LayoutType = "ShowAllInRow",
		SelectType = "SelectOne",
		OneChoiceForAllPlayers = true,
		ExportOnChange = false,
		Choices = { 'Off','On' },
		LoadSelections = function(self, list, pn)
			if ReadPrefFromFile("UserPrefComboOnRolls") ~= nil then
				if GetUserPrefB("UserPrefComboOnRolls") then
					list[2] = true;
				else
					list[1] = true;
				end;
			else
				WritePrefToFile("UserPrefComboOnRolls",false);
				list[1] = true;
			end;
		end,
		SaveSelections = function(self, list, pn)
			local val = list[2] and true or false
			WritePrefToFile("UserPrefComboOnRolls", val)
			MESSAGEMAN:Broadcast("PreferenceSet", { Message == "Set Preference" })
			THEME:ReloadMetrics()
		end
	}
	setmetatable(t, t)
	return t
end

function UserPrefFlashyCombo()
	local t = {
		Name = "UserPrefFlashyCombo",
		LayoutType = "ShowAllInRow",
		SelectType = "SelectOne",
		OneChoiceForAllPlayers = true,
		ExportOnChange = false,
		Choices = { 'Off','On' },
		LoadSelections = function(self, list, pn)
			if ReadPrefFromFile("UserPrefFlashyCombo") ~= nil then
				if GetUserPrefB("UserPrefFlashyCombo") then
					list[2] = true
				else
					list[1] = true
				end
			else
				WritePrefToFile("UserPrefFlashyCombo", false)
				list[1] = true
			end
		end,
		SaveSelections = function(self, list, pn)
			local val = list[2] and true or false
			WritePrefToFile("UserPrefFlashyCombo", val)
			MESSAGEMAN:Broadcast("PreferenceSet", { Message == "Set Preference" })
			THEME:ReloadMetrics()
		end
	}
	setmetatable(t, t)
	return t
end

function UserPrefComboUnderField()
	local t = {
		Name = "UserPrefComboUnderField",
		LayoutType = "ShowAllInRow",
		SelectType = "SelectOne",
		OneChoiceForAllPlayers = true,
		ExportOnChange = false,
		Choices = { 'Off','On' },
		LoadSelections = function(self, list, pn)
			if ReadPrefFromFile("UserPrefComboUnderField") ~= nil then
				if GetUserPrefB("UserPrefComboUnderField") then
					list[2] = true
				else
					list[1] = true
				end
			else
				WritePrefToFile("UserPrefComboUnderField", true)
				list[2] = true
			end
		end,
		SaveSelections = function(self, list, pn)
			local val = list[2] and true or false
			WritePrefToFile("UserPrefComboUnderField", val)
			MESSAGEMAN:Broadcast("PreferenceSet", { Message == "Set Preference" })
			THEME:ReloadMetrics()
		end
	}
	setmetatable(t, t)
	return t;
end

function UserPrefFancyUIBG()
	local t = {
		Name = "UserPrefFancyUIBG",
		LayoutType = "ShowAllInRow",
		SelectType = "SelectOne",
		OneChoiceForAllPlayers = true,
		ExportOnChange = false,
		Choices = {
			THEME:GetString('OptionNames','On'),
			THEME:GetString('OptionNames','Off')
		},
		LoadSelections = function(self, list, pn)
			if ReadPrefFromFile("UserPrefFancyUIBG") ~= nil then
				if GetUserPrefB("UserPrefFancyUIBG") then
					list[1] = true
				else
					list[2] = true
				end
			else
				WritePrefToFile("UserPrefFancyUIBG", true)
				list[1] = true
			end
		end,
		SaveSelections = function(self, list, pn)
			local val = list[1] and true or false
			WritePrefToFile("UserPrefFancyUIBG", val)
			MESSAGEMAN:Broadcast("PreferenceSet", { Message == "Set Preference" })
			THEME:ReloadMetrics()
		end
	}
	setmetatable(t, t)
	return t
end

--[[ end option rows ]]
