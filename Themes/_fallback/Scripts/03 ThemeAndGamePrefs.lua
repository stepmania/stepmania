-- sm-ssc Default Theme Preferences Handler
function InitGamePrefs()
	local Prefs = 
	{
	};

	local BPrefs =
	{
		{ "ComboOnRolls",	false },
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
	if GetUserPrefB("ShowLotsaOptions") == nil then
		SetUserPref("ShowLotsaOptions", true);
	end;

	local Prefs =
	{
		{ "GameplayShowStepsDisplay",	true },
		{ "GameplayShowScore",		false},
--[[		{ "ProTimingP1",	false},
		{ "ProTimingP2",	false},
--]]
	};

	local BPrefs = 
	{
		{ "AutoSetStyle",		false },
		{ "ComboUnderField",	true },
		{ "LongFail",			false},
		{ "NotePosition",		true },
		{ "UserPrefProtimingP1",		false},
		{ "UserPrefProtimingP2",		false},
		{ "ShowLotsaOptions",	true},
		{ "ComboOnRolls",	false},
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
	return t;
end;

function GetDefaultOptionLines()
	local LineSets = {
		"1,8,14,2,3,4,5,6,R,7,9,10,11,12,13,15,16,SF,17,18", -- All
		"1,8,14,2,7,13,16,SF,17,18", -- DDR Essentials ( no turns, fx )
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
		return "1,8,14,2,7,13,16,SF,17,18" -- "failsafe" list
	end
end;

--[[ end themeoption rows ]]

--[[ game option rows ]]
