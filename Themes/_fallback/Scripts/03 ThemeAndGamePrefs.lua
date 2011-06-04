-- sm-ssc Default Theme Preferences Handler
function InitGamePrefs()
	local Prefs = 
	{
		{ "DefaultFail",	"Immediate" },
	};
	
	local BPrefs =
	{
		{ "AutoSetStyle",	false },
		{ "NotePosition",	true },
		{ "ComboOnRolls",	false },
		{ "ComboUnderField",	true },
	};

	for idx,pref in ipairs(Prefs) do
		if GetGamePref( pref[1] ) == nil then
			SetGamePref( pref[1], pref[2] );
		end;
	end;
	
	for idx,pref in ipairs(BPrefs) do
		if GetGamePrefB( pref[1] ) == nil then
			SetGamePref( pref[1], pref[2] );
		end;
	end;
end
function InitUserPrefs()
	if GetUserPrefB("UserPrefShowLotsaOptions") == nil then
		SetUserPref("UserPrefShowLotsaOptions", true);
	end;

	local Prefs =
	{
		{ "UserPrefGameplayShowStepsDisplay",	true },
		{ "UserPrefGameplayShowScore",		false},
--[[		{ "ProTimingP1",	false},
		{ "ProTimingP2",	false},
--]]
	};		

	local BPrefs = 
	{
		{ "UserPrefShowLotsaOptions",		true},
		{ "UserPrefLongFail",			false},
		{ "UserPrefProtimingP1",		false},
		{ "UserPrefProtimingP2",		false},
		{ "FlashyCombos",	false},
		{ "GameplayFooter",	false},
	};

	for idx,pref in ipairs(Prefs) do
		if GetUserPref( pref[1] ) == nil then
			SetUserPref( pref[1], pref[2] );
		end;
	end;

	-- making sure I don't screw up anything yet...
	for idx,pref in ipairs(BPrefs) do
		if GetUserPrefB( pref[1] ) == nil then
			SetUserPref( pref[1], pref[2] );
		end;
	end;

end;
--[[ theme option rows ]]
-- screen cover
function GetProTiming(pn)
	local pname = ToEnumShortString(pn);
	if GetUserPref("ProTiming"..pname) then
		return GetUserPrefB("ProTiming"..pname);
	else
		SetUserPref("ProTiming"..pname,false);
		return false;
	end;
end;
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
		"1,8,14,2,3,4,5,6,R,7,9,10,11,12,13,15,16,17,18", -- All
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

function UserPrefGameplayFooter()
	local t = {
		Name = "UserPrefGameplayFooter";
		LayoutType = "ShowAllInRow";
		SelectType = "SelectOne";
		OneChoiceForAllPlayers = true;
		ExportOnChange = false;
		Choices = { 'Off','On' };
		LoadSelections = function(self, list, pn)
			if ReadPrefFromFile("GameplayFooter") ~= nil then
				if GetUserPrefB("GameplayFooter") then
					list[2] = true;
				else
					list[1] = true;
				end;
			else
				WritePrefToFile("GameplayFooter",false);
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
			WritePrefToFile("GameplayFooter",val);
			MESSAGEMAN:Broadcast("PreferenceSet", { Message == "Set Preference" } );
			THEME:ReloadMetrics();
		end;
	};
	setmetatable( t, t );
	return t;
end
--[[ end themeoption rows ]]

--[[ game option rows ]]
function GamePrefComboOnRolls()
	local t = {
		Name = "GamePrefComboOnRolls";
		LayoutType = "ShowAllInRow";
		SelectType = "SelectOne";
		OneChoiceForAllPlayers = true;
		ExportOnChange = false;
		Choices = { 'Off','On' };
		LoadSelections = function(self, list, pn)
			if ReadGamePrefFromFile("ComboOnRolls") ~= nil then
				if GetGamePrefB("ComboOnRolls") then
					list[2] = true;
				else
					list[1] = true;
				end;
			else
				WriteGamePrefToFile("ComboOnRolls",false);
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
			WriteGamePrefToFile("ComboOnRolls",val);
			MESSAGEMAN:Broadcast("PreferenceSet", { Message == "Set Preference" } );
			THEME:ReloadMetrics();
		end;
	};
	setmetatable( t, t );
	return t;
end
function GamePrefComboUnderField()
	local t = {
		Name = "GamePrefComboUnderField";
		LayoutType = "ShowAllInRow";
		SelectType = "SelectOne";
		OneChoiceForAllPlayers = true;
		ExportOnChange = false;
		Choices = { 'Off','On' };
		LoadSelections = function(self, list, pn)
			if ReadGamePrefFromFile("ComboUnderField") ~= nil then
				if GetGamePrefB("ComboUnderField") then
					list[2] = true;
				else
					list[1] = true;
				end;
			else
				WriteGamePrefToFile("ComboUnderField",true);
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
			WriteGamePrefToFile("ComboUnderField",val);
			MESSAGEMAN:Broadcast("PreferenceSet", { Message == "Set Preference" } );
			THEME:ReloadMetrics();
		end;
	};
	setmetatable( t, t );
	return t;
end

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
function GamePrefNotePosition()
	local t = {
		Name = "GamePrefNotePosition";
		LayoutType = "ShowAllInRow";
		SelectType = "SelectOne";
		OneChoiceForAllPlayers = true;
		ExportOnChange = false;
		Choices = { 'Normal','Lower' };
		LoadSelections = function(self, list, pn)
			if ReadGamePrefFromFile("NotePosition") ~= nil then
				if GetGamePrefB("NotePosition") then
					list[1] = true;
				else
					list[2] = true;
				end;
			else
				WriteGamePrefToFile("NotePosition",false);
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
			WriteGamePrefToFile("NotePosition",val);
			MESSAGEMAN:Broadcast("PreferenceSet", { Message == "Set Preference" } );
			THEME:ReloadMetrics();
		end;
	};
	setmetatable( t, t );
	return t;
end

function GamePrefDefaultFail()
	local t = {
		Name = "GamePrefDefaultFail";
		LayoutType = "ShowAllInRow";
		SelectType = "SelectOne";
		OneChoiceForAllPlayers = true;
		ExportOnChange = false;
		Choices = { "Immediate","ImmediateContinue", "AtEnd", "Off" };
		LoadSelections = function(self, list, pn)
			if ReadGamePrefFromFile("DefaultFail") ~= nil then
				if GetGamePref("DefaultFail") then
					if GetGamePref("DefaultFail") == "Immediate" then
						list[1] = true;
					elseif GetGamePref("DefaultFail") == "ImmediateContinue" then
						list[2] = true;
					elseif GetGamePref("DefaultFail") == "AtEnd" then
						list[3] = true;
					elseif GetGamePref("DefaultFail") == "Off" then
						list[4] = true;
					else
						list[1] = true;
					end
					-- list[table.find( list, GetGamePref("DefaultFail") )] = true;
				else
					list[1] = true;
				end;
			else
				WriteGamePrefToFile("DefaultFail","Immediate");
				list[1] = true;
			end;
		end;
		SaveSelections = function(self, list, pn)
			-- This is so stupid.
			local tChoices = { "Immediate","ImmediateContinue", "AtEnd", "Off" };
			local val;
			if list[1] then
				val = tChoices[1];
			elseif list[2] then
				val = tChoices[2];
			elseif list[3] then
				val = tChoices[3];
			elseif list[4] then
				val = tChoices[4];
			else
				val = tChoices[1];
			end
			WriteGamePrefToFile("DefaultFail",val);
			MESSAGEMAN:Broadcast("PreferenceSet", { Message == "Set Preference" } );
			THEME:ReloadMetrics();
		end;
	};
	setmetatable( t, t );
	return t;
end
