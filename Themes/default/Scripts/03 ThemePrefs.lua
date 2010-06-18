-- sm-ssc Default Theme Preferences Handler

function InitUserPrefs()
	if GetUserPref("UserPrefGameplayShowStepsDisplay") == nil then
		SetUserPref("UserPrefGameplayShowStepsDisplay", true);
	end;
	if GetUserPref("UserPrefGameplayShowScore") == nil then
		SetUserPref("UserPrefGameplayShowScore", true);
	end;
	if GetUserPrefB("UserPrefShowLotsaOptions") == nil then
		SetUserPref("UserPrefShowLotsaOptions", true);
	end;  
	if GetUserPrefB("UserPrefAutoSetStyle") == nil then
		SetUserPref("UserPrefAutoSetStyle", true);
	end;  
	if GetUserPrefB("UserPrefLongFail") == nil then
		SetUserPref("UserPrefLongFail", false);
	end;  
	if GetUserPrefB("UserPrefNotePosition") == nil then
		SetUserPref("UserPrefNotePosition", true);
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
			local pname = ToEnumShortString(pn);

			if getenv("ProTiming"..pname) == true then
				list[2] = true;
			else
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
			local pname = ToEnumShortString(pn);
			setenv("ProTiming"..pname, val);
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
			MESSAGEMAN:Broadcast("PrferenceSet", { Message == "Set Preference" } );
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
			MESSAGEMAN:Broadcast("PrferenceSet", { Message == "Set Preference" } );
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
			MESSAGEMAN:Broadcast("PrferenceSet", { Message == "Set Preference" } );
			THEME:ReloadMetrics();	
		end;
	};
	setmetatable( t, t );
	return t;
end

function GetDefaultOptionLines()
	local LineSets = {
		"1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17", -- All
		"1,2,7,8,13,14,16,17", -- DDR Essentials ( no turns, fx )
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
		return "1,2,7,8,13,14,17" -- "failsafe" list
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
			MESSAGEMAN:Broadcast("PrferenceSet", { Message == "Set Preference" } );
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
			MESSAGEMAN:Broadcast("PrferenceSet", { Message == "Set Preference" } );
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
			MESSAGEMAN:Broadcast("PrferenceSet", { Message == "Set Preference" } );
			THEME:ReloadMetrics();	
		end;
	};
	setmetatable( t, t );
	return t;
end
--[[ end option rows ]]