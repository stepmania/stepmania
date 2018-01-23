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


local function speed_type_menu_item(tab)
	return {"item", tab, "speed_type", "choice", {choices= notefield_speed_types, translation_section= "notefield_options"}}
end

local function speed_mod_menu_item(tab)
	return {"item", tab, "speed_mod", "large_number", {translation_section= "notefield_options"}}
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
	return {
		"item", "generic", "noteskin", "choice", {
			choices= skin_names, dont_translate_value= true,
			set= function(arg, choice, pn)
				set_func(config, choice)
				set_skin_for_field(field, field_name, stepstype)
				MESSAGEMAN:Broadcast("NoteskinChanged", {field_name})
			end,
			get= function(arg, pn)
				return get_func(config, stepstype)
			end,
	}}
end

local function editor_noteskin_param_menu(field, field_name, stepstype)
	local choices= {}
	local config_data= editor_config:get_data()
	local skin_names= NOTESKIN:get_skin_names_for_stepstype(stepstype)
	for i, skin_name in ipairs(skin_names) do
		choices[#choices+1]= {
			"custom", {
				name= skin_name, dont_translate_name= true,
				type_hint= {main= "submenu", sub= "noteskin"},
				func= function(big, arg, pn)
					local skin_info= NOTESKIN:get_skin_parameter_info(skin_name)
					local skin_defaults= NOTESKIN:get_skin_parameter_defaults(skin_name)
					local chosen_params= config_data:get_skin_params(skin_name)
					local items= noteskin_params_menu_level(chosen_params, skin_info, skin_defaults)
					return "submenu", nesty_menus.add_close_item(items)
				end,
				on_close= function()
					if field:get_skin() == skin_name then
						set_skin_for_field(field, field_name, stepstype)
						MESSAGEMAN:Broadcast("NoteskinChanged", {field:GetName()})
					end
				end,
		}}
	end
	return {"submenu", "noteskin_params", choices, translation_section= "notefield_options"}
end

local function editor_menu_options(field, field_name, stepstype)
	local config= editor_config:get_data()[field_name]
	if not config then
		lua.ReportScriptError("No editor config table for notefield '" .. tostring(field_name) .. "'")
		return nil
	end
	local items= {
		{"item", "song_option", "MusicRate"},
		editor_noteskin_menu(field, field_name, stepstype),
		editor_noteskin_param_menu(field, field_name, stepstype),
		{"item", config, "hidden", "bool"},
		{"item", config, "hidden_offset", "number"},
		{"item", config, "sudden", "bool"},
		{"item", config, "sudden_offset", "number"},
		{"item", config, "fade_dist", "number"},
		{"item", config, "glow_during_fade", "bool"},
		{"item", config, "fov", "number"},
		{"item", config, "reverse", "toggle_number", {on= -1, off= 1}},
		{"item", config, "rotation_x", "number"},
		{"item", config, "rotation_y", "number"},
		{"item", config, "rotation_z", "number"},
		{"item", config, "yoffset", "number"},
		{"item", config, "zoom", "percent"},
		{"item", config, "zoom_x", "percent"},
		{"item", config, "zoom_y", "percent"},
		{"item", config, "zoom_z", "percent"},
	}
	if config.speed_type then
		table.insert(items, 1, speed_mod_menu_item(config))
		table.insert(items, 2, speed_type_menu_item(config))
	end
	return nesty_menus.add_close_item(items, THEME:GetString("editmode_options", "edit_return"), true)
end


function editor_notefield_menu(menu_actor)
	menu_actor.Name= "menu"
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
	local menu= setmetatable({}, menu_controller_mt)
	local sound_actors= false

	local prev_mx= INPUTFILTER:GetMouseX()
	local prev_my= INPUTFILTER:GetMouseY()
	local function update()
		local mx= INPUTFILTER:GetMouseX()
		local my= INPUTFILTER:GetMouseY()
		if mx ~= prev_mx or my ~= prev_my then
			sound_name= menu:update_focus(mx, my)
			nesty_menus.play_menu_sound(sounds, sound_name)
			prev_mx= mx
			prev_my= my
		end
	end

	local function input(event)
		if not in_option_menu then return end
		local levels_left, sound_name= menu:input(event)
		nesty_menus.play_menu_sound(sound_actors, sound_name)
		if levels_left and levels_left < 1 then
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
			sound_actors= nesty_menus.make_menu_sound_lookup(self)
			container= self
			editor_screen= SCREENMAN:GetTopScreen()
			editor_screen:AddInputCallback(input)
			self:SetUpdateFunction(update)
			menu:init{actor= self:GetChild("menu"), input_mode= "four_direction",
								translation_section= "editmode_options",
								repeats_to_big= 10, select_goes_to_top= true}
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
			menu:set_info(menu_options)
			menu:open_menu()
			self:queuecommand("set_in")
		end,
		set_inCommand= function(self)
			in_option_menu= true
		end,
		MenuValueChangedMessageCommand= function(self, params)
			apply_notefield_prefs_nopn(current_read_bpm, current_field, current_field_options)
			local vispix= 1024 / current_field_options.zoom
			for i, col in ipairs(current_field:get_columns()) do
				col:set_pixels_visible_before(vispix)
				col:set_pixels_visible_after(vispix)
			end
		end,
		menu_actor,
	}
end
