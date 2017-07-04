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

nesty_option_menus.shown_noteskins= {
	type= "menu",
	__index= {
		initialize= function(self, pn, extra)
			self.pn= pn
			self.name= extra.name
			self.all_noteskin_names= NOTESKIN:get_all_skin_names()
			self.shown_config= shown_noteskins:get_data(self.pn)
			self.info_set= {nesty_menu_up_element}
			for i, skin_name in ipairs(self.all_noteskin_names) do
				local show= not self.shown_config[skin_name]
				self.info_set[#self.info_set+1]= {text= skin_name, type= "bool", value= show}
			end
			self.cursor_pos= 1
		end,
		destructor= function(self)
			shown_noteskins:save(self.pn)
		end,
		set_status= function(self)
			self.display:set_heading(self.name)
			self.display:set_status("")
		end,
		interpret_start= function(self)
			local info= self.info_set[self.cursor_pos]
			shown_noteskins:set_dirty(self.pn)
			local skin_name= info.text
			self.shown_config[skin_name]= not self.shown_config[skin_name]
			info.value= not self.shown_config[skin_name]
			self:update_el_value(self.cursor_pos, info.value)
			return true
		end,
}}

nesty_option_menus.noteskins= {
	type= "menu",
	__index= {
		scroll_to_move_on_start= true,
		initialize= function(self, pn)
			self.pn= pn
			self.cursor_pos= 1
			self.stepstype= find_current_stepstype(pn)
			self.ops= filter_noteskin_list_with_shown_config(
				pn, NOTESKIN:get_skin_names_for_stepstype(self.stepstype))
			self.player_skin= PROFILEMAN:GetProfile(pn):get_preferred_noteskin(self.stepstype)
			local function find_matching_noteskin()
				for ni, nv in ipairs(self.ops) do
					--Trace("Noteskin found: '" .. tostring(nv) .. "'")
					if self.player_skin == nv then
						return ni
					end
				end
				return nil
			end
			self.selected_skin= find_matching_noteskin()
			self.info_set= {nesty_menu_up_element}
			for ni, nv in ipairs(self.ops) do
				self.info_set[#self.info_set+1]= {
					text= nv, value= ni == self.selected_skin, type= "choice"}
			end
		end,
		interpret_start= function(self)
			local ops_pos= self.cursor_pos - 1
			local info= self.info_set[self.cursor_pos]
			if self.ops[ops_pos] then
				for i, tinfo in ipairs(self.info_set) do
					if i ~= self.cursor_pos and tinfo.value then
						self:update_el_value(i, false)
					end
				end
				self.player_skin= self.ops[ops_pos]
				PROFILEMAN:GetProfile(self.pn):set_preferred_noteskin(self.player_skin)
				self:update_el_value(self.cursor_pos, true)
				self:set_status()
				MESSAGEMAN:Broadcast("NoteskinChanged", {pn= self.pn})
				return true
			else
				return false
			end
		end,
		set_status= function(self)
			self.display:set_heading("Noteskin")
			self.display:set_status(self.player_skin)
		end
}}

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
local function int_val_text(pn, val)
	return ("%d"):format(val)
end
local function float_val_text(pn, val)
	return ("%.2f"):format(val)
end
local function noteskin_param_float_val(param_name, param_section, type_info, default_val)
	local min_scale= 0
	local max_scale= 0
	local val_text= int_val_text
	if type_info.type ~= "int" then
		min_scale= -2
		val_text= float_val_text
	end
	local val_min= type_info.min
	local val_max= type_info.max
	if type_info.max then
		max_scale= math.floor(math.log(type_info.max) / math.log(10))
	else
		max_scale= 2
	end
	local translation= get_noteskin_param_translation(param_name, type_info)
	return {
		name= translation.title, menu= nesty_option_menus.adjustable_float,
		explanation= translation.explanation,
		value= function() return param_section[param_name] end,
		args= {
			name= translation.title, min_scale= min_scale, scale= 0,
			max_scale= max_scale, reset_value= default_val,
			initial_value= function()
				return param_section[param_name]
			end,
			set= function(pn, value) param_section[param_name]= value end,
			val_to_text= val_to_text, val_min= val_min, val_max= val_max,
	}}
end
local function noteskin_param_choice_val(param_name, param_section, type_info)
	local eles= {}
	local translation= get_noteskin_param_translation(param_name, type_info)
	for i, choice in ipairs(type_info.choices) do
		eles[#eles+1]= {
			type= "choice",
			name= translation.choices[i], explanation= translation.explanation,
			execute= function(pn)
				param_section[param_name]= choice
			end,
			value= function(pn)
				return param_section[param_name] == choice
			end,
		}
	end
	return {name= translation.title, explanation= translation.explanation,
					value= param_section[param_name],
					args= eles, menu= nesty_option_menus.menu}
end
local function noteskin_param_bool_val(param_name, param_section, type_info)
	local translation= get_noteskin_param_translation(param_name, type_info)
	return {
		type= "bool",
		name= translation.title, explanation= translation.explanation,
		translatable= false, execute= function(pn)
			param_section[param_name]= not param_section[param_name]
		end,
		value= function(pn)
			return param_section[param_name]
		end,
	}
end
function gen_noteskin_param_submenu(param_section, type_info, skin_defaults, add_to)
	for field, info in pairs(type_info) do
		if field ~= "translation" then
			local field_type= type(skin_defaults[field])
			local translation= get_noteskin_param_translation(field, type_info[field])
			local submenu= {name= translation.title, explanation= translation.explanation}
			if param_section[field] == nil then
				if field_type == "table" then
					param_section[field]= DeepCopy(skin_defaults[field])
				else
					param_section[field]= skin_defaults[field]
				end
			end
			if field_type == "table" then
				submenu.menu= nesty_option_menus.menu
				local menu_args= {}
				gen_noteskin_param_submenu(param_section[field], info, skin_defaults[field], menu_args)
				submenu.args= menu_args
			elseif field_type == "string" then
				if info.choices then
					submenu= noteskin_param_choice_val(field, param_section, info)
				else
					submenu= nil
				end
			elseif field_type == "number" then
				if info.choices then
					submenu= noteskin_param_choice_val(field, param_section, info)
				else
					submenu= noteskin_param_float_val(field, param_section, info, skin_defaults[field])
				end
			elseif field_type == "boolean" then
				submenu= noteskin_param_bool_val(field, param_section, info)
			end
			add_to[#add_to+1]= submenu
		end
	end
	local function submenu_cmp(a, b)
		return a.name < b.name
	end
	table.sort(add_to, submenu_cmp)
end
function gen_noteskin_param_menu(pn)
	local stepstype= find_current_stepstype(pn)
	local profile= PROFILEMAN:GetProfile(pn)
	local player_skin= profile:get_preferred_noteskin(stepstype)
	local skin_info= NOTESKIN:get_skin_parameter_info(player_skin)
	local skin_defaults= NOTESKIN:get_skin_parameter_defaults(player_skin)
	local player_params= profile:get_noteskin_params(player_skin)
	if not player_params then
		player_params= {}
		profile:set_noteskin_params(player_skin, player_params)
	end
	local ret= {
		recall_init_on_pop= true,
		name= "noteskin_params",
		destructor= function(self, pn)
			MESSAGEMAN:Broadcast("NoteskinParamsChanged", {pn= pn})
		end,
	}
	gen_noteskin_param_submenu(player_params, skin_info, skin_defaults, ret)
	return ret
end
function show_noteskin_param_menu(pn)
	local stepstype= find_current_stepstype(pn)
	local player_skin= PROFILEMAN:GetProfile(pn):get_preferred_noteskin(stepstype)
	local skin_info= NOTESKIN:get_skin_parameter_info(player_skin)
	local skin_defaults= NOTESKIN:get_skin_parameter_defaults(player_skin)
	return skin_defaults ~= nil and skin_info ~= nil and next(skin_defaults)
end
set_nesty_option_metatables()


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
