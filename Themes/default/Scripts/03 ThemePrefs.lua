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
};

ThemePrefs.InitAll( Prefs )

function InitUserPrefs()
	if GetUserPref("UserPrefGameplayShowStepsDisplay") == nil then
		SetUserPref("UserPrefGameplayShowStepsDisplay", true);
	end;
	if GetUserPref("UserPrefGameplayShowScore") == nil then
		SetUserPref("UserPrefGameplayShowScore", false);
	end;
	if GetUserPref("UserPrefScoringMode") == nil then
		SetUserPref("UserPrefScoringMode", 'DDR Extreme');
	end;
	if GetUserPrefB("UserPrefShowLotsaOptions") == nil then
		SetUserPref("UserPrefShowLotsaOptions", true);
	end;
	if GetUserPrefB("UserPrefAutoSetStyle") == nil then
		SetUserPref("UserPrefAutoSetStyle", false);
	end;
	if GetUserPrefB("UserPrefLongFail") == nil then
		SetUserPref("UserPrefLongFail", false);
	end;
	if GetUserPrefB("UserPrefNotePosition") == nil then
		SetUserPref("UserPrefNotePosition", true);
	end;
	if GetUserPrefB("UserPrefComboOnRolls") == nil then
		SetUserPref("UserPrefComboOnRolls", false);
	end;
	if GetUserPrefB("UserPrefProtimingP1") == nil then
		SetUserPref("UserPrefProtimingP1", false);
	end;
	if GetUserPrefB("UserPrefProtimingP2") == nil then
		SetUserPref("UserPrefProtimingP2", false);
	end;
	if GetUserPrefB("FlashyCombos") == nil then
		SetUserPref("FlashyCombos", false);
	end;
	if GetUserPrefB("UserPrefComboUnderField") == nil then
		SetUserPref("UserPrefComboUnderField", true);
	end;
--[[ 	if GetUserPref("ProTimingP1") == nil then
		SetUserPref("ProTimingP1", false);
	end;
	if GetUserPref("ProTimingP2") == nil then
		SetUserPref("ProTimingP2", false);
	end; --]]
end;

function GetProTiming(pn)
	local pname = ToEnumShortString(pn);
	if GetUserPref("ProTiming"..pname) then
		return GetUserPrefB("ProTiming"..pname);
	else
		SetUserPref("ProTiming"..pname,false);
		return false;
	end;
end;

--[[ option rows ]]

-- screen cover
function OptionRowProTiming()
	local t = {
		Name = "ProTiming";
		LayoutType = "ShowAllInRow";
		SelectType = "SelectOne";
		OneChoiceForAllPlayers = false;
		ExportOnChange = false;
		Choices = { 'Off','On' };
		LoadSelections = function(self, list, pn)
			local bShow;
			if GetUserPrefB("UserPrefProtiming" .. ToEnumShortString(pn) ) then
				bShow = GetUserPrefB("UserPrefProtiming" .. ToEnumShortString(pn) );
				if bShow then
					list[2] = true;
				else
					list[1] = true;
				end
			else
				list[1] = true;
			end;
--[[ 			local pname = ToEnumShortString(pn);

			if getenv("ProTiming"..pname) == true then
				list[2] = true;
			else
				list[1] = true;
			end; --]]
		end;
		SaveSelections = function(self, list, pn)
			local bSave;
			if list[2] then
				bSave = true;
			else
				bSave = false;
			end;
			SetUserPref("UserPrefProtiming" .. ToEnumShortString(pn),bSave);
--[[ 			local val;
			if list[2] then
				val = true;
			else
				val = false;
			end;
			local pname = ToEnumShortString(pn);
			setenv("ProTiming"..pname, val); --]]
		end;
	};
	setmetatable( t, t );
	return t;
end;

function UserPrefGameplayShowScore()
	local t = {
		Name = "UserPrefGameplayShowScore";
		LayoutType = "ShowAllInRow";
		SelectType = "SelectOne";
		OneChoiceForAllPlayers = true;
		ExportOnChange = false;
		Choices = { 'Off','On' };
		LoadSelections = function(self, list, pn)
			if ReadPrefFromFile("UserPrefGameplayShowScore") ~= nil then
				if GetUserPrefB("UserPrefGameplayShowScore") then
					list[2] = true;
				else
					list[1] = true;
				end;
			else
				WritePrefToFile("UserPrefGameplayShowScore",false);
				list[1] = true;
			end;
		end;
		SaveSelections = function(self, list, pn)
			local val;
			if list[2] then
				val = true;
			else
				val = false;
			end;
			WritePrefToFile("UserPrefGameplayShowScore",val);
			MESSAGEMAN:Broadcast("PreferenceSet", { Message == "Set Preference" } );
			THEME:ReloadMetrics();
		end;
	};
	setmetatable( t, t );
	return t;
end

function UserPrefGameplayShowStepsDisplay()
	local t = {
		Name = "UserPrefGameplayShowStepsDisplay";
		LayoutType = "ShowAllInRow";
		SelectType = "SelectOne";
		OneChoiceForAllPlayers = true;
		ExportOnChange = false;
		Choices = { 'Off','On' };
		LoadSelections = function(self, list, pn)
			if ReadPrefFromFile("UserPrefGameplayShowStepsDisplay") ~= nil then
				if GetUserPrefB("UserPrefGameplayShowStepsDisplay") then
					list[2] = true;
				else
					list[1] = true;
				end;
			else
				WritePrefToFile("UserPrefGameplayShowStepsDisplay",false);
				list[1] = true;
			end;
		end;
		SaveSelections = function(self, list, pn)
			local val;
			if list[2] then
				val = true;
			else
				val = false;
			end;
			WritePrefToFile("UserPrefGameplayShowStepsDisplay",val);
			MESSAGEMAN:Broadcast("PreferenceSet", { Message == "Set Preference" } );
			THEME:ReloadMetrics();
		end;
	};
	setmetatable( t, t );
	return t;
end

function UserPrefShowLotsaOptions()
	local t = {
		Name = "UserPrefShowLotsaOptions";
		LayoutType = "ShowAllInRow";
		SelectType = "SelectOne";
		OneChoiceForAllPlayers = true;
		ExportOnChange = false;
		Choices = { 'Many','Few' };
		LoadSelections = function(self, list, pn)
			if ReadPrefFromFile("UserPrefShowLotsaOptions") ~= nil then
				if GetUserPrefB("UserPrefShowLotsaOptions") then
					list[1] = true;
				else
					list[2] = true;
				end;
			else
				WritePrefToFile("UserPrefShowLotsaOptions",false);
				list[2] = true;
			end;
		end;
		SaveSelections = function(self, list, pn)
			local val;
			if list[1] then
				val = true;
			else
				val = false;
			end;
			WritePrefToFile("UserPrefShowLotsaOptions",val);
			MESSAGEMAN:Broadcast("PreferenceSet", { Message == "Set Preference" } );
			THEME:ReloadMetrics();
		end;
	};
	setmetatable( t, t );
	return t;
end

function GetDefaultOptionLines()
	local LineSets = {
		"1,8,14,2,3,4,5,6,R,7,9,10,11,12,13,16,17,18", -- All
		"1,8,14,2,7,13,16,17,18", -- DDR Essentials ( no turns, fx )
	};
	local function IsExtra()
		if GAMESTATE:IsExtraStage() or GAMESTATE:IsExtraStage2() then
			return true
		else
			return false
		end
	end
	if not IsExtra() then
		if GetUserPrefB("UserPrefShowLotsaOptions") then
			return GetUserPrefB("UserPrefShowLotsaOptions") and LineSets[1] or LineSets[2];
		else
			return LineSets[2]; -- Just make sure!
		end
	else
		return "1,8,14,2,7,13,16,17,18" -- "failsafe" list
	end
end;

function UserPrefAutoSetStyle()
	local t = {
		Name = "UserPrefAutoSetStyle";
		LayoutType = "ShowAllInRow";
		SelectType = "SelectOne";
		OneChoiceForAllPlayers = true;
		ExportOnChange = false;
		Choices = { 'Off','On' };
		LoadSelections = function(self, list, pn)
			if ReadPrefFromFile("UserPrefAutoSetStyle") ~= nil then
				if GetUserPrefB("UserPrefAutoSetStyle") then
					list[2] = true;
				else
					list[1] = true;
				end;
			else
				WritePrefToFile("UserPrefAutoSetStyle",false);
				list[1] = true;
			end;
		end;
		SaveSelections = function(self, list, pn)
			local val;
			if list[2] then
				val = true;
			else
				val = false;
			end;
			WritePrefToFile("UserPrefAutoSetStyle",val);
			MESSAGEMAN:Broadcast("PreferenceSet", { Message == "Set Preference" } );
			THEME:ReloadMetrics();
		end;
	};
	setmetatable( t, t );
	return t;
end


function UserPrefNotePosition()
	local t = {
		Name = "UserPrefNotePosition";
		LayoutType = "ShowAllInRow";
		SelectType = "SelectOne";
		OneChoiceForAllPlayers = true;
		ExportOnChange = false;
		Choices = { 'Normal','Lower' };
		LoadSelections = function(self, list, pn)
			if ReadPrefFromFile("UserPrefNotePosition") ~= nil then
				if GetUserPrefB("UserPrefNotePosition") then
					list[1] = true;
				else
					list[2] = true;
				end;
			else
				WritePrefToFile("UserPrefNotePosition",false);
				list[1] = true;
			end;
		end;
		SaveSelections = function(self, list, pn)
			local val;
			if list[1] then
				val = true;
			else
				val = false;
			end;
			WritePrefToFile("UserPrefNotePosition",val);
			MESSAGEMAN:Broadcast("PreferenceSet", { Message == "Set Preference" } );
			THEME:ReloadMetrics();
		end;
	};
	setmetatable( t, t );
	return t;
end

function UserPrefLongFail()
	local t = {
		Name = "UserPrefLongFail";
		LayoutType = "ShowAllInRow";
		SelectType = "SelectOne";
		OneChoiceForAllPlayers = true;
		ExportOnChange = false;
		Choices = { 'Short','Long' };
		LoadSelections = function(self, list, pn)
			if ReadPrefFromFile("UserPrefLongFail") ~= nil then
				if GetUserPrefB("UserPrefLongFail") then
					list[2] = true;
				else
					list[1] = true;
				end;
			else
				WritePrefToFile("UserPrefLongFail",false);
				list[1] = true;
			end;
		end;
		SaveSelections = function(self, list, pn)
			local val;
			if list[2] then
				val = true;
			else
				val = false;
			end;
			WritePrefToFile("UserPrefLongFail",val);
			MESSAGEMAN:Broadcast("PreferenceSet", { Message == "Set Preference" } );
			THEME:ReloadMetrics();
		end;
	};
	setmetatable( t, t );
	return t;
end

function UserPrefComboOnRolls()
	local t = {
		Name = "UserPrefComboOnRolls";
		LayoutType = "ShowAllInRow";
		SelectType = "SelectOne";
		OneChoiceForAllPlayers = true;
		ExportOnChange = false;
		Choices = { 'Off','On' };
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
		end;
		SaveSelections = function(self, list, pn)
			local val;
			if list[2] then
				val = true;
			else
				val = false;
			end;
			WritePrefToFile("UserPrefComboOnRolls",val);
			MESSAGEMAN:Broadcast("PreferenceSet", { Message == "Set Preference" } );
			THEME:ReloadMetrics();
		end;
	};
	setmetatable( t, t );
	return t;
end

function UserPrefFlashyCombo()
	local t = {
		Name = "UserPrefFlashyCombo";
		LayoutType = "ShowAllInRow";
		SelectType = "SelectOne";
		OneChoiceForAllPlayers = true;
		ExportOnChange = false;
		Choices = { 'Off','On' };
		LoadSelections = function(self, list, pn)
			if ReadPrefFromFile("UserPrefFlashyCombo") ~= nil then
				if GetUserPrefB("UserPrefFlashyCombo") then
					list[2] = true;
				else
					list[1] = true;
				end;
			else
				WritePrefToFile("UserPrefFlashyCombo",false);
				list[1] = true;
			end;
		end;
		SaveSelections = function(self, list, pn)
			local val;
			if list[2] then
				val = true;
			else
				val = false;
			end;
			WritePrefToFile("UserPrefFlashyCombo",val);
			MESSAGEMAN:Broadcast("PreferenceSet", { Message == "Set Preference" } );
			THEME:ReloadMetrics();
		end;
	};
	setmetatable( t, t );
	return t;
end

function UserPrefComboUnderField()
	local t = {
		Name = "UserPrefComboUnderField";
		LayoutType = "ShowAllInRow";
		SelectType = "SelectOne";
		OneChoiceForAllPlayers = true;
		ExportOnChange = false;
		Choices = { 'Off','On' };
		LoadSelections = function(self, list, pn)
			if ReadPrefFromFile("UserPrefComboUnderField") ~= nil then
				if GetUserPrefB("UserPrefComboUnderField") then
					list[2] = true;
				else
					list[1] = true;
				end;
			else
				WritePrefToFile("UserPrefComboUnderField",true);
				list[2] = true;
			end;
		end;
		SaveSelections = function(self, list, pn)
			local val;
			if list[2] then
				val = true;
			else
				val = false;
			end;
			WritePrefToFile("UserPrefComboUnderField",val);
			MESSAGEMAN:Broadcast("PreferenceSet", { Message == "Set Preference" } );
			THEME:ReloadMetrics();
		end;
	};
	setmetatable( t, t );
	return t;
end
--[[ end option rows ]]
