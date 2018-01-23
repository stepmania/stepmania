shown_noteskins= create_lua_config{
	name= "shown noteskins", file= "shown_noteskins.lua", default= {},
	match_depth= 0, use_global_as_default= true,
	use_alternate_config_prefix= "",
}
shown_noteskins:load()

add_standard_lua_config_save_load_hooks(shown_noteskins)

-- The config actually stores true for noteskins that need to be hidden,
-- because that seems more resilient to the player adding and removing
-- noteskins from the data folders.

function filter_noteskin_list_with_shown_config(pn, skin_list)
	local config= shown_noteskins:get_data(pn)
	local skinny= 1
	while skinny <= #skin_list do
		if config[skin_list[skinny]] then
			table.remove(skin_list, skinny)
		else
			skinny= skinny + 1
		end
	end
	return skin_list
end

function find_current_stepstype(pn)
	local steps= GAMESTATE:GetCurrentSteps(pn)
	if steps then
		return steps:GetStepsType()
	end
	local style= GAMESTATE:GetCurrentStyle(pn)
	if style then
		return style:GetStepsType()
	end
	style= GAMEMAN:GetStylesForGame(GAMESTATE:GetCurrentGame():GetName())[1]
	if style then
		return style:GetStepsType()
	end
	local last_type= PROFILEMAN:GetProfile(pn):get_last_stepstype()
	if last_type then
		return last_type
	end
	return "StepsType_Dance_Single"
end

function shown_noteskins_menu()
	return {
		"custom", {
			name= "shown_noteskins", type_hint= {main= "submenu", sub= "noteskin"},
			translation_section= "notefield_options",
			func= function(big, arg, pn)
				local items= {}
				local all_noteskin_names= NOTESKIN:get_all_skin_names()
				local config= shown_noteskins:get_data(pn)
				for i, skin_name in ipairs(all_noteskin_names) do
					items[#items+1]= {
						name= skin_name, func_changes_value= true,
						dont_translate_name= true,
						func= function()
							config[skin_name]= not config[skin_name]
							shown_noteskins:set_dirty(pn)
							return {"boolean", not config[skin_name]}
						end,
						value= function()
							return {"boolean", not config[skin_name]}
						end,
					}
				end
				nesty_menus.add_close_item(items)
				return "submenu", items
	end}}
end

local function generate_noteskin_menu_items(pn)
	local items= {}
	local stepstype= find_current_stepstype(pn)
	local skins= filter_noteskin_list_with_shown_config(
		pn, NOTESKIN:get_skin_names_for_stepstype(stepstype))
	for i, skin_name in ipairs(skins) do
		items[#items+1]= {
			name= skin_name, dont_translate_name= true,
			translation_section= "notefield_options",
			refresh= {category= "NoteSkin"},
			func= function()
				PROFILEMAN:GetProfile(pn):set_preferred_noteskin(skin_name)
				nesty_menus.menu_message{
					category= "NoteSkin", pn= pn, value= skin_name}
				MESSAGEMAN:Broadcast("NoteskinChanged", {pn= pn, skin= skin_name})
			end,
			value= function()
				local player_skin= PROFILEMAN:GetProfile(pn)
					:get_preferred_noteskin(stepstype)
				if skin_name == player_skin then
					return {"boolean", true}
				end
				return nil
			end,
		}
	end
	nesty_menus.add_close_item(items)
	return items
end

function noteskin_menu_item()
	return {
		"custom", {
			name= "noteskin", type_hint= {main= "choice", sub= "noteskin"},
			translation_section= "notefield_options",
			dont_translate_value= true,
			refresh= {category= "NoteSkin"},
			value= function(arg, pn)
				return {"string", PROFILEMAN:GetProfile(pn):get_preferred_noteskin(find_current_stepstype(pn))}
			end,
			adjust= function(dir, big, arg, pn)
				local stype= find_current_stepstype(pn)
				local skins= filter_noteskin_list_with_shown_config(
					pn, NOTESKIN:get_skin_names_for_stepstype(stype))
				local player_skin= PROFILEMAN:GetProfile(pn)
					:get_preferred_noteskin(stype)
				local id= 0
				for i, entry in ipairs(skins) do
					if entry == player_skin then id= i break end
				end
				id= (((id + dir) - 1) % #skins) + 1
				player_skin= skins[id]
				PROFILEMAN:GetProfile(pn):set_preferred_noteskin(player_skin)
				nesty_menus.menu_message{
					category= "NoteSkin", pn= pn, value= player_skin}
				MESSAGEMAN:Broadcast("NoteskinChanged", {pn= pn})
				return {"string", player_skin}
			end,
			func= function(big, arg, pn)
				return "submenu", generate_noteskin_menu_items(pn)
			end,
	}}
end

local function get_noteskin_param_translation(param_name, type_info)
	local translation_table= type_info.translation
	local ret= {title= param_name, explanation= param_name}
	if type(translation_table) == "table" then
		local language= PREFSMAN:GetPreference("Language")
		if translation_table[language] then
			ret= translation_table[language]
		else
			local base_language= "en"
			if translation_table[base_language] then
				ret= translation_table[base_language]
			else
				local lang, text= next(translation_table)
				if text then ret= text end
			end
		end
	end
	if type_info.choices and not ret.choices then
		ret.choices= type_info.choices
	end
	return ret
end

function noteskin_params_menu_level(player_params, type_info, skin_defaults)
	local items= {}
	for field, info in pairs(type_info) do
		local field_type= type(skin_defaults[field])
		local translation= get_noteskin_param_translation(field, type_info[field])
		if player_params[field] == nil then
			if field_type == "table" then
				player_params[field]= DeepCopy(skin_defaults[field])
			else
				player_params[field]= skin_defaults[field]
			end
		end
		if field_type == "table" then
			items[#items+1]= {
				name= translation.title, explanation= translation.explanation,
				type_hint= {main= "submenu", sub= "noteskin"},
				dont_translate_name= true, func= function(big, arg, pn)
					local sub_items= noteskin_params_menu_level(
						player_params[field], info, skin_defaults[field])
					return "submenu", nesty_menus.add_close_item(sub_items)
				end,
			}
		elseif field_type == "string" then
			local choices= {}
			for i, choice in ipairs(info.choices) do
				choices[#choices+1]= {translation.choices[i], choice}
			end
			items[#items+1]= nesty_menus.item(
				player_params, translation.title, "name_value_pairs", {
					reset= skin_defaults[field],
					choices= choices, explanation= translation.explanation,
					dont_translate_name= true, dont_translate_value= true, path= field})
		elseif field_type == "number" then
			local val_min= info.min
			local val_max= info.max
			local small_step= 1
			if info.type ~= "int" then
				small_step= .01
			end
			items[#items+1]= nesty_menus.item(
				player_params, translation.title, "number", {
					reset= skin_defaults[field], small_step= small_step,
					big_step= small_step * 10, path= field,
					dont_translate_name= true, dont_translate_value= true})
		elseif field_type == "boolean" then
			items[#items+1]= nesty_menus.item(
				player_params, translation.title, "bool", {
					reset= skin_defaults[field], path= field,
					dont_translate_name= true, dont_translate_value= true})
		end
	end
	local function item_cmp(a, b)
		return a.name < b.name
	end
	table.sort(items, item_cmp)
	return items
end

function noteskin_params_menu_item()
	return {
		"custom", {
			name= "noteskin_params", type_hint= {main= "submenu", sub= "noteskin"},
			translation_section= "notefield_options",
			func= function(big, arg, pn)
				local stepstype= find_current_stepstype(pn)
				local profile= PROFILEMAN:GetProfile(pn)
				local player_skin= profile:get_preferred_noteskin(stepstype)
				local skin_info= NOTESKIN:get_skin_parameter_info(player_skin)
				local skin_defaults= NOTESKIN:get_skin_parameter_defaults(player_skin)
				if not skin_defaults or not skin_info then return end
				local player_params= profile:get_noteskin_params(player_skin)
				if not player_params then
					player_params= {}
					profile:set_noteskin_params(player_skin, player_params)
				end
				local items= noteskin_params_menu_level(
					player_params, skin_info, skin_defaults)
				if #items < 1 then return end
				return "submenu", nesty_menus.add_close_item(items)
			end,
			on_close= function(arg, pos, pn)
				MESSAGEMAN:Broadcast("NoteskinParamsChanged", {pn= pn})
			end,
	}}
end


function noteskin_option_row()
	local pn= GAMESTATE:GetMasterPlayerNumber()
	local steps= GAMESTATE:GetCurrentSteps(pn)
	if not steps then
		steps= GAMESTATE:GetCurrentTrail(pn)
	end
	local stype= false
	if steps then
		stype= steps:GetStepsType()
	elseif not GAMESTATE:InStepEditor() then
		local profile= PROFILEMAN:GetProfile(pn)
		stype= profile:get_last_stepstype()
	end
	if not stype then
		return {
			Name= "NoteSkin",
			GoToFirstOnStart= true,
			LayoutType= "ShowAllInRow",
			SelectType= "SelectOne",
			Choices= {""},
			LoadSelections= function() end,
			SaveSelections= function() end,
		}
	end
	local skins= NOTESKIN:get_skin_names_for_stepstype(stype)
	if #skins < 1 then
		return {
			Name= "NoteSkin",
			GoToFirstOnStart= true,
			LayoutType= "ShowAllInRow",
			SelectType= "SelectOne",
			Choices= {""},
			LoadSelections= function() end,
			SaveSelections= function() end,
		}
	end
	return {
		Name= "NoteSkin",
		GoToFirstOnStart= true,
		LayoutType= "ShowAllInRow",
		SelectType= "SelectOne",
		Choices= skins,
		LoadSelections= function(self, list, pn)
			local profile= PROFILEMAN:GetProfile(pn)
			local player_skin= profile:get_preferred_noteskin(stype)
			for i, choice in ipairs(self.Choices) do
				if player_skin == choice then
					list[i]= true
					return
				end
			end
			list[1]= true
		end,
		SaveSelections= function(self, list, pn)
			for i, choice in ipairs(self.Choices) do
				if list[i] then
					local profile= PROFILEMAN:GetProfile(pn)
					local player_skin= profile:set_preferred_noteskin(choice)
					return
				end
			end
		end,
	}
end
