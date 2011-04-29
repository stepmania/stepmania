--[[--------------------------------------------------------------------------
A fresh and simple way to set up OptionRows -DaisuMaster
--]]--------------------------------------------------------------------------

--[[ CORE FUNCTIONS ]]--
--{ Those must be internal only }--

-- Main template
local function OptionRowTemplate(name,layout,select,onechoice,export,choices,loadfunc,savefunc)
	local t = {
		Name = name;
		LayoutType = layout;
		SelectType = select;
		OneChoiceForAllPlayers = onechoice;
		ExportOnChange = export;
		Choices = choices;
		LoadSelections = loadfunc;
		SaveSelections = savefunc;
	}
	setmetatable( t, t )
	return t
end

-- Create templates for UserPreferences
local function OptionRowUserPrefTemplate(name,choices)
	local function load(self, list, pn)
		if GetUserPrefB(name) then
			list[1] = true
		else
			list[2] = true
		end
	end
	local function save(self, list, pn)
		local bool
		if list[1] then
			bool = true
		else
			bool = false
		end
		SetUserPref(name,bool)
	end
	return OptionRowTemplate(name,"ShowAllInRow","SelectOne",true,false,choices,load,save)
end

-- Same as above but for EnvUtils
local function OptionRowEnvutilsTemplate(name,onechoice,choices)
	local function load(self, list, pn)
		if onechoice then
			setenv(name,false)
		else
			setenv(name..pn,false)
		end
		list[1] = true
	end
	local function save(self, list, pn)
		local bool
		if list[1] then
			bool = false
		else
			bool = true
		end
		if onechoice then
			setenv(name,bool)
		else
			setenv(name..pn,bool)
		end
	end
	if not choices then
		choices = {
			THEME:GetString("OptionNames","Off"),
			THEME:GetString("OptionNames","On"),
		}
	end
	return OptionRowTemplate(name,"ShowAllInRow","SelectOne",onechoice,false,choices,load,save)
end

--[[ whatever else you want/like put it there (keep it clean please) ]]--
--{ Those should be used externally }--

function OptionReverseGrade()
	return OptionRowEnvutilsTemplate("GradeReverse",false)
end

function UserPrefOptionsMode()
	local choices = {
		THEME:GetString("OptionNames","OptionsList"),
		THEME:GetString("OptionNames","Codes"),
	}
	return OptionRowUserPrefTemplate("OptionsMode", choices)
end