local default_config= {
	recently_edited= {},
	recent_groups= {},
	edit_noteskin= {},
	test_noteskin= {},
	noteskin_params= {},
	NoteFieldEdit= {
		hidden= false,
		hidden_offset= 120,
		sudden= false,
		sudden_offset= 190,
		fade_dist= 40,
		glow_during_fade= true,
		fov= 45,
		reverse= 1,
		rotation_x= 0,
		rotation_y= 0,
		rotation_z= 0,
		vanish_x= 0,
		vanish_y= 0,
		yoffset= 130,
		zoom= .5,
		zoom_x= 1,
		zoom_y= 1,
		zoom_z= 1,
	},
	NoteFieldTest= {
		speed_mod= 250,
		speed_type= "maximum",
		hidden= false,
		hidden_offset= 120,
		sudden= false,
		sudden_offset= 190,
		fade_dist= 40,
		glow_during_fade= true,
		fov= 45,
		reverse= 1,
		rotation_x= 0,
		rotation_y= 0,
		rotation_z= 0,
		vanish_x= 0,
		vanish_y= 0,
		yoffset= 130,
		zoom= 1,
		zoom_x= 1,
		zoom_y= 1,
		zoom_z= 1,
	},
	preview= {
		show_preview= false,
		min_offset= -1,
		max_offset= 1,
		playback_speed= 1,
		paused= false,
		mods_reload_key= 'KP 0',
		pause_key= 'KP 2',
		hide_key= 'KP 1',
		full_screen_key= 'KP 3',
		move_key= 'KP 6',
		smaller_key= 'KP 4',
		larger_key= 'KP 5',
		zoom= .25,
		corner= 3,
	},
}

editor_config= create_lua_config{
	name= "editor_config", file= "editor_config.lua",
	default= default_config, use_alternate_config_prefix= "",
	exceptions= {"recently_edited", "recent_groups", "edit_noteskin",
		"test_noteskin", "noteskin_params"},
}

editor_config:load()

local function get_skin_choice(list, stepstype)
	local valid_skin_names= NOTESKIN:get_skin_names_for_stepstype(stepstype)
	for i= 1, #list do
		local match= list[i]
		for v= 1, #valid_skin_names do
			if match == valid_skin_names[v] then
				return match
			end
		end
	end
	return valid_skin_names[1]
end

local function set_skin_choice(list, choice)
	local found= false
	for i= 1, #list do
		if list[i] == choice then
			found= i
			break
		end
	end
	if found then
		table.remove(list, found)
	end
	table.insert(list, 1, choice)
end

local function sanity_check_editor_config()
	local config_data= editor_config:get_data()
	for s, skin_list in pairs{config_data.edit_noteskin, config_data.test_noteskin} do
		for i, name in pairs(skin_list) do
			if type(i) ~= "number" then
				skin_list[i]= nil
			end
		end
	end
	for skin_name, params in pairs(config_data.noteskin_params) do
		if type(params) ~= "table" or type(skin_name) ~= "string" then
			config_data.noteskin_params[skin_name]= nil
		end
	end
	local entry_id= #config_data.recently_edited
	while entry_id > 0 do
		local entry= config_data.recently_edited[entry_id]
		if type(entry.group) ~= "string"
			or type(entry.song_dir) ~= "string"
			or type(entry.stepstype) ~= "string"
			or type(entry.difficulty) ~= "string"
		or type(entry.description) ~= "string" then
			table.remove(config_data.recently_edited, entry_id)
		end
		entry_id= entry_id - 1
	end
	entry_id= #config_data.recent_groups
	while entry_id > 0 do
		local entry= config_data.recent_groups[entry_id]
		if type(entry) ~= "string" then
			table.remove(config_data.recent_groups, entry_id)
		end
		entry_id= entry_id - 1
	end

	config_data.get_edit_skin_choice= function(self, stepstype)
		return get_skin_choice(self.edit_noteskin, stepstype)
	end
	config_data.get_test_skin_choice= function(self, stepstype)
		return get_skin_choice(self.test_noteskin, stepstype)
	end
	config_data.set_edit_skin_choice= function(self, choice)
		set_skin_choice(self.edit_noteskin, choice)
	end
	config_data.set_test_skin_choice= function(self, choice)
		set_skin_choice(self.test_noteskin, choice)
	end
	config_data.get_skin_params= function(self, skin_name)
		if not self.noteskin_params[skin_name] then
			self.noteskin_params[skin_name]= {}
		end
		return self.noteskin_params[skin_name]
	end
end

local function check_all_match(left, right)
	for field, value in pairs(left) do
		if value ~= right[field] then return false end
	end
	return true
end

local function remove_duplicate(container, original, dup_id, dup)
	if check_all_match(original, dup) then
		table.remove(container, dup_id)
		return true
	end
	return false
end

sanity_check_editor_config()

function add_to_recent_groups(group_name)
	local recent= editor_config:get_data().recent_groups
	local old_id= #recent
	while old_id > 0 do
		if recent[old_id] == group_name then
			table.remove(recent, old_id)
		end
		old_id= old_id - 1
	end
	table.insert(recent, 1, group_name)
	editor_config:set_dirty()
end

function add_to_recently_edited(song, steps)
	-- TODO: When the user edits the chart description or deletes the chart,
	-- the entry needs to be updated. -Kyz
	local new_entry= {
		group= song:GetGroupName(),
		song_dir= song:GetSongDir(),
		stepstype= steps:GetStepsType(),
		difficulty= steps:GetDifficulty(),
		description= steps:GetDescription(),
	}
	local recent= editor_config:get_data().recently_edited
	local old_id= #recent
	while old_id > 0 do
		remove_duplicate(recent, new_entry, old_id, recent[old_id])
		old_id= old_id -1
	end
	table.insert(recent, 1, new_entry)
	editor_config:set_dirty()
	editor_config:save()
end

local function set_skin_for_field(field, field_name, stepstype)
	local config_data= editor_config:get_data()
	local list= config_data.edit_noteskin
	if field_name == "NoteFieldTest" then
		list= config_data.test_noteskin
	end
	local skin_name= get_skin_choice(list, stepstype)
	local skin_params= config_data:get_skin_params(skin_name)
	field:set_skin(skin_name, skin_params)
end


local function use_multiple_speed_mod(tab)
	return function()
		return tab.speed_type == "multiple"
	end
end

local function not_use_multiple(tab)
	return function()
		return tab.speed_type ~= "multiple"
	end
end

local function speed_type_menu(tab)
	return setmetatable({
			name= "speed_type", menu= nesty_option_menus.enum_option,
			translatable= true, value= function()
				return tab.speed_type
			end,
			args= {
				name= "speed_type", enum= notefield_speed_types, fake_enum= true,
				obj_get= function() return tab end,
				get= function(pn, obj) return obj.speed_type end,
				set= function(pn, obj, value)
					if obj.speed_type == "multiple" and value ~= "multiple" then
						obj.speed_mod= math.round(obj.speed_mod * 100)
					elseif obj.speed_type ~= "multiple" and value == "multiple" then
						obj.speed_mod= obj.speed_mod / 100
					end
					obj.speed_type= value
					MESSAGEMAN:Broadcast(
						"ConfigValueChanged", {field_name= "speed_type", value= value})
				end,
											}}, mergable_table_mt)
end

local function editor_noteskin_menu(field, field_name, stepstype)
	local skin_names= NOTESKIN:get_skin_names_for_stepstype(stepstype)
	local config= editor_config:get_data()
	local get_func= config.get_edit_skin_choice
	local set_func= config.set_edit_skin_choice
	if field_name == "NoteFieldTest" then
		get_func= config.get_test_skin_choice
		set_func= config.set_test_skin_choice
	end
	local choices= {}
	for i, name in ipairs(skin_names) do
		choices[#choices+1]= {
			type= "choice", name= name, execute= function()
				set_func(config, name)
				set_skin_for_field(field, field_name, stepstype)
				MESSAGEMAN:Broadcast("NoteskinChanged", {field_name})
			end,
			value= function()
				return name == get_func(config, stepstype)
			end,
		}
	end
	return nesty_options.submenu("noteskin", choices)
end

local function editor_noteskin_param_menu(field, stepstype)
	local choices= {}
	local config_data= editor_config:get_data()
	local skin_names= NOTESKIN:get_skin_names_for_stepstype(stepstype)
	for i, skin_name in ipairs(skin_names) do
		choices[#choices+1]= {
			name= skin_name, menu= nesty_option_menus.menu, args= function()
				local skin_info= NOTESKIN:get_skin_parameter_info(skin_name)
				local skin_defaults= NOTESKIN:get_skin_parameter_defaults(skin_name)
				local chosen_params= config_data:get_skin_params(skin_name)
				ret= {
					recall_init_on_pop= true, name= "noteskin_params",
					destructor= function(self)
						if field:get_skin() == skin_name then
							set_skin_for_field(field, stepstype)
							MESSAGEMAN:Broadcast("NoteskinChanged", {field:GetName()})
						end
					end,
				}
				gen_noteskin_param_submenu(chosen_params, skin_info, skin_defaults, ret)
				return ret
		end}
	end
	return nesty_options.submenu("noteskin_params", choices)
end

local function editor_menu_options(field, field_name, stepstype)
	local config= editor_config:get_data()[field_name]
	if not config then
		lua.ReportScriptError("No editor config table for notefield '" .. tostring(field_name) .. "'")
		return nil
	end
	local ret= {
		nesty_options.float_song_mod_val("MusicRate", -2, -1, -1, .5, 2, 1),
		editor_noteskin_menu(field, field_name, stepstype),
		editor_noteskin_param_menu(field, stepstype),
		nesty_options.bool_table_val(field_name, config, "hidden"),
		nesty_options.float_table_val(field_name, config, "hidden_offset", -1, 1, 2),
		nesty_options.bool_table_val(field_name, config, "sudden"),
		nesty_options.float_table_val(field_name, config, "sudden_offset", -1, 1, 2),
		nesty_options.float_table_val(field_name, config, "fade_dist", -1, 1, 2),
		nesty_options.bool_table_val(field_name, config, "glow_during_fade"),
		nesty_options.float_table_val(field_name, config, "fov", -1, 0, 1, 1, 179),
		nesty_options.float_table_val(field_name, config, "reverse", -2, 0, 0),
		nesty_options.float_table_val(field_name, config, "rotation_x", -1, 1, 2),
		nesty_options.float_table_val(field_name, config, "rotation_y", -1, 1, 2),
		nesty_options.float_table_val(field_name, config, "rotation_z", -1, 1, 2),
		nesty_options.float_table_val(field_name, config, "yoffset", -1, 1, 2),
		nesty_options.float_table_val(field_name, config, "zoom", -2, -2, 0),
		nesty_options.float_table_val(field_name, config, "zoom_x", -2, -1, 0),
		nesty_options.float_table_val(field_name, config, "zoom_y", -2, -1, 0),
		nesty_options.float_table_val(field_name, config, "zoom_z", -2, -1, 0),
	}
	if config.speed_type then
		table.insert(ret, 1,
			nesty_options.float_table_val(field_name, config, "speed_mod", -2, -1, 1) ..
			 {req_func= use_multiple_speed_mod(config)})
		table.insert(ret, 2,
			nesty_options.float_table_val(field_name, config, "speed_mod", 0, 1, 3) ..
			 {req_func= not_use_multiple(config)})
		table.insert(ret, 3, speed_type_menu(config))
	end
	return setmetatable(ret, mergable_table_mt)
end

function editor_notefield_menu(menu_params)
	local editor_screen= false
	local in_option_menu= false
	local current_field= false
	local current_read_bpm= 140
	local current_field_options= false
	local current_stepstype= false
	local edit_field= false
	local test_field= false
	local field_name= ""
	local container= false
	local menu= setmetatable({}, nesty_menu_stack_mt)

	local function input(event)
		if not in_option_menu then return end
		if event.type == "InputEventType_Release" then return end
		local button= event.GameButton
		if not button then return end
		local menu_action= menu:interpret_code(button)
		if menu_action == "close" then
			editor_config:set_dirty()
			editor_config:save()
			in_option_menu= false
			editor_screen:PostScreenMessage("SM_BackFromNoteFieldOptions", 0)
			if container:GetParent() ~= editor_screen then
				container:GetParent():playcommand("HideMenu")
			end
		end
	end

	return Def.ActorFrame{
		OnCommand= function(self)
			container= self
			editor_screen= SCREENMAN:GetTopScreen()
			editor_screen:AddInputCallback(input)
			menu:hide()
		end,
		InitNoteFieldConfigCommand= function(self, params)
			local conf_data= editor_config:get_data()
			edit_field= params.fields.NoteFieldEdit
			test_field= params.fields.NoteFieldTest

			for i, info in ipairs{
				{edit_field, conf_data.edit_noteskin},
				{test_field, conf_data.test_noteskin}} do
				local skin= get_skin_choice(info[2], params.stepstype)
				local params= conf_data:get_skin_params(skin)
				info[1]:set_skin(skin, params)
			end

			current_read_bpm= params.read_bpm
			for field_name, field in pairs(params.fields) do
				apply_notefield_prefs_nopn(params.read_bpm, field, conf_data[field_name])
				local vispix= 1024 / conf_data[field_name].zoom
				for i, col in ipairs(field:get_columns()) do
					col:set_pixels_visible_before(vispix)
					col:set_pixels_visible_after(vispix)
				end
			end
			for i, col in ipairs(edit_field:get_columns()) do
				col:set_speed_segments_enabled(false)
				col:set_scroll_segments_enabled(false)
			end
			MESSAGEMAN:Broadcast("NoteskinChanged")
		end,
		SetTestNoteFieldSkinCommand= function(self, params)
			set_skin_for_field(params.field, "NoteFieldTest", params.stepstype)
		end,
		ShowMenuCommand= function(self, params)
			current_field= params.field
			field_name= params.field_name
			current_stepstype= params.stepstype
			local conf_data= editor_config:get_data()
			local list= conf_data.edit_noteskin
			if field_name == "NoteFieldTest" then
				list= conf_data.test_noteskin
			end
			local skin= get_skin_choice(list, params.stepstype)
			if current_field:get_skin() ~= skin then
				local skin_params= conf_data:get_skin_params(skin)
				params.field:set_skin(skin, skin_params)
			end
			local menu_options= editor_menu_options(current_field, field_name, params.stepstype)
			if not menu_options then
				lua.ReportScriptError("Unable to generate menu options?")
				editor_screen:PostScreenMessage("SM_BackFromNoteFieldOptions", 0)
				return
			end
			current_field_options= editor_config:get_data()[field_name]
			menu:clear_menu_stack()
			menu:push_menu_stack(nesty_option_menus.menu, menu_options, "edit_return")
			menu:unhide()
			self:queuecommand("set_in")
		end,
		set_inCommand= function(self)
			in_option_menu= true
		end,
		ConfigValueChangedMessageCommand= function(self, params)
			apply_notefield_prefs_nopn(current_read_bpm, current_field, current_field_options)
			local vispix= 1024 / current_field_options.zoom
			for i, col in ipairs(current_field:get_columns()) do
				col:set_pixels_visible_before(vispix)
				col:set_pixels_visible_after(vispix)
			end
		end,
		menu:create_actors(menu_params),
	}
end
