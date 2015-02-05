-- All other languages are compared to the master language to determine what is
-- missing and what is unused.
-- The list of missing strings for a language is saved to lang_missing.ini,
-- where lang is the name of the language.
-- The list of unused strings is saved to lang_unused.ini.

function find_missing_strings_in_theme_translations(theme_name, master_name)
	local lang_folder= "Themes/"..theme_name.."/Languages/"
	local master_language= IniFile.ReadFile(lang_folder .. master_name)
	local other_languages= {}
	local language_names= FILEMAN:GetDirListing(lang_folder)
	-- Load all languages.
	for i, name in ipairs(language_names) do
		if name ~= master_name and not name:find("missing")
		and not name:find("unused") then
			other_languages[#other_languages+1]= {
				name= name, data= IniFile.ReadFile(lang_folder .. name)}
		end
	end
	-- Find out what is missing from each language.
	local missing_list= {}
	local function find_missing_in_section(section_name, section)
		local function add_str_to_missing(str_name, str_value)
			for i, other in ipairs(other_languages) do
				if not other.data[section_name] or
				not other.data[section_name][str_name] then
					if not missing_list[other.name] then
						missing_list[other.name]= {}
					end
					if not missing_list[other.name][section_name] then
						missing_list[other.name][section_name]= {}
					end
					missing_list[other.name][section_name][str_name]= str_value
				end
			end
		end
		foreach_ordered(section, add_str_to_missing)
	end
	foreach_ordered(master_language, find_missing_in_section)
	local function save_missing_data(lang_name, data)
		IniFile.WriteFile(
			lang_folder .. lang_name:sub(1, -5) .. "_missing.ini", data)
	end
	foreach_ordered(missing_list, save_missing_data)
	-- Find out what is extra in each language.
	for i, other in ipairs(other_languages) do
		local unused= {}
		local function find_unused_in_section(section_name, section)
			local function add_str_to_unused(str_name, str_value)
				if not master_language[section_name] or
				not master_language[section_name][str_name] then
					if not unused[section_name] then
						unused[section_name]= {}
					end
					unused[section_name][str_name]= str_value
				end
			end
			foreach_ordered(section, add_str_to_unused)
		end
		foreach_ordered(other.data, find_unused_in_section)
		IniFile.WriteFile(
			lang_folder .. other.name:sub(1, -5) .. "_unused.ini", unused)
	end
end
