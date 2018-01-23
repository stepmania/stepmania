local notefield_default_prefs= {
	speed_step= 10,
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
	-- migrated_from_newfield_name is temporary, remove it in 5.1.-4. -Kyz
	migrated_from_newfield_name= false,
}

notefield_speed_types= {"maximum", "constant", "multiple"}

-- If the theme author uses Ctrl+F2 to reload scripts, the config that was
-- loaded from the player's profile will not be reloaded.
-- But the old instance of notefield_prefs_config still exists, so the data
-- from it can be copied over.  The config system has a function for handling
-- this.
notefield_prefs_config= create_lua_config{
	name= "notefield_prefs", file= "notefield_prefs.lua",
	default= notefield_default_prefs,
	-- use_alternate_config_prefix is meant for lua configs that are shared
	-- between multiple themes.  It should be nil for preferences that will
	-- only exist in your theme. -Kyz
	use_alternate_config_prefix= "",
}

add_standard_lua_config_save_load_hooks(notefield_prefs_config)

local function migrate_from_newfield_name(profile, dir, pn)
	if not pn then return end
	local config_data= notefield_prefs_config:get_data(pn)
	if not config_data.migrated_from_newfield_name then
		local newfield_config_path= dir .. "/newfield_prefs.lua"
		if FILEMAN:DoesFileExist(newfield_config_path) then
			local newfield_config= lua.load_config_lua(newfield_config_path)
			if type(newfield_config) == "table" then
				for field, value in pairs(newfield_config) do
					config_data[field]= value
				end
			end
		end
		config_data.migrated_from_newfield_name= true
		notefield_prefs_config:set_dirty(pn)
		notefield_prefs_config:save(pn)
	end
end
add_profile_load_callback(migrate_from_newfield_name)

function set_notefield_default_yoffset(yoff)
	notefield_default_prefs.yoffset= yoff
end

function apply_notefield_prefs_nopn(read_bpm, field, prefs)
	local torad= 1 / 180
	if prefs.speed_type then
		if prefs.speed_type == "maximum" then
			field:set_speed_mod(false, prefs.speed_mod, read_bpm)
		elseif prefs.speed_type == "constant" then
			field:set_speed_mod(true, prefs.speed_mod)
		else
			field:set_speed_mod(false, prefs.speed_mod/100)
		end
	end
	field:set_base_values{
		fov_x= prefs.vanish_x,
		fov_y= prefs.vanish_y,
		fov_z= prefs.fov,
		transform_rot_x= prefs.rotation_x*torad,
		transform_rot_y= prefs.rotation_y*torad,
		transform_rot_z= prefs.rotation_z*torad,
		transform_zoom= prefs.zoom,
		transform_zoom_x= prefs.zoom_x,
		transform_zoom_y= prefs.zoom_y,
		transform_zoom_z= prefs.zoom_z,
	}
	-- Use the y zoom to adjust the y offset to put the receptors in the same
	-- place.
	local adjusted_offset= prefs.yoffset / (prefs.zoom * prefs.zoom_y)
	for i, col in ipairs(field:get_columns()) do
		col:set_base_values{
			reverse= prefs.reverse,
			reverse_offset= adjusted_offset,
		}
	end
	if prefs.hidden then
		field:set_hidden_mod(prefs.hidden_offset, prefs.fade_dist, prefs.glow_during_fade)
	else
		field:clear_hidden_mod()
	end
	if prefs.sudden then
		field:set_sudden_mod(prefs.sudden_offset, prefs.fade_dist, prefs.glow_during_fade)
	else
		field:clear_sudden_mod()
	end
end

function apply_notefield_prefs(pn, field, prefs)
	local pstate= GAMESTATE:GetPlayerState(pn)
	apply_notefield_prefs_nopn(pstate:get_read_bpm(), field, prefs)
	local poptions= pstate:get_player_options_no_defect("ModsLevel_Song")
	if prefs.speed_type == "maximum" then
		poptions:MMod(prefs.speed_mod, 1000)
	elseif prefs.speed_type == "constant" then
		poptions:CMod(prefs.speed_mod, 1000)
	else
		poptions:XMod(prefs.speed_mod/100, 1000)
	end
	local reverse= scale(prefs.reverse, 1, -1, 0, 1)
	poptions:Reverse(reverse, 1000)
	-- -1 tilt = +30 rotation_x
	local tilt= prefs.rotation_x / -30
	if prefs.reverse < 0 then
		tilt = tilt * -1
	end
	poptions:Tilt(tilt, 1000)
	local mini= (1 - prefs.zoom) * 2, 1000
	if tilt > 0 then
		mini = mini * scale(tilt, 0, 1, 1, .9)
	else
		mini = mini * scale(tilt, 0, -1, 1, .9)
	end
	poptions:Mini(mini, 1000)
	local steps= GAMESTATE:GetCurrentSteps(pn)
	if steps and steps:HasAttacks() then
		pstate:set_needs_defective_field(true)
	end
	if GAMESTATE:IsCourseMode() then
		local course= GAMESTATE:GetCurrentCourse()
		if course and course:HasMods() or course:HasTimedMods() then
			pstate:set_needs_defective_field(true)
		end
	end
end

function find_field_apply_prefs(pn)
	local screen_gameplay= SCREENMAN:GetTopScreen()
	local field= find_notefield_in_gameplay(screen_gameplay, pn)
	if field then
		apply_notefield_prefs(pn, field, notefield_prefs_config:get_data(pn))
	end
end

function notefield_prefs_actor()
	return Def.Actor{
		OnCommand= function(self)
			for pn in ivalues(GAMESTATE:GetEnabledPlayers()) do
				find_field_apply_prefs(pn)
			end
		end,
		CurrentStepsP1ChangedMessageCommand= function(self, param)
			if not GAMESTATE:GetCurrentSteps(PLAYER_1) then return end
			-- In course mode, the steps change message is broadcast before the
			-- field and other things are given the new note data.  So delay
			-- reapplying the prefs until next frame to make sure they take effect
			-- after the steps are fully changed. -Kyz
			self:queuecommand("delayed_p1_steps_change")
		end,
		CurrentStepsP2ChangedMessageCommand= function(self, param)
			if not GAMESTATE:GetCurrentSteps(PLAYER_2) then return end
			self:queuecommand("delayed_p2_steps_change")
		end,
		delayed_p1_steps_changeCommand= function(self)
			find_field_apply_prefs(PLAYER_1)
		end,
		delayed_p2_steps_changeCommand= function(self)
			find_field_apply_prefs(PLAYER_2)
		end,
		MenuValueChangedMessageCommand= function(self, params)
			if params.pn then
				if params.config_name == "notefield_prefs" then
					find_field_apply_prefs(params.pn)
				elseif params.category == "NoteSkin" then
					local pn= params.pn
					local skin= params.value
					local field= find_notefield_in_gameplay(SCREENMAN:GetTopScreen(), pn)
					if field then
						local profile= PROFILEMAN:GetProfile(pn)
						local skin_params= profile:get_noteskin_params(skin)
						field:set_skin(skin, skin_params)
					end
				end
			end
		end,
	}
end

function reset_needs_defective_field_for_all_players()
	for i, pn in ipairs{PLAYER_1, PLAYER_2} do
		GAMESTATE:GetPlayerState(pn):set_needs_defective_field(false)
	end
end

function adv_notefield_prefs_menu()
	local items= {}
	local info= {
		{"hidden_offset", "number"},
		{"sudden_offset", "number"},
		{"hidden"},
		{"sudden"},
		{"fade_dist", "number"},
		{"glow_during_fade"},
		{"reverse", "percent"},
		{"zoom", "percent"},
		{"rotation_x", "number"},
		{"rotation_y", "number"},
		{"rotation_z", "number"},
		{"vanish_x", "number"},
		{"vanish_y", "number"},
		{"fov", "number", min= 1, max= 179},
		{"yoffset", "number"},
		{"zoom_x", "percent"},
		{"zoom_y", "percent"},
		{"zoom_z", "percent"},
	}
	for i, entry in ipairs(info) do
		if #entry == 1 then
			items[#items+1]= {"item", notefield_prefs_config, entry[1], "bool", {translation_section= "notefield_options"}}
		else
			items[#items+1]= {"item", notefield_prefs_config, entry[1], entry[2], {min= entry.min, max= entry.max, translation_section= "notefield_options"}}
		end
	end
	return {"submenu", "advanced_notefield_config", items, translation_section= "notefield_options"}
end

local function speed_mod_ret(data, value)
	if get_element_by_path(data, "speed_type") == "multiple" then
		return {"number", value / 100}
	else
		return {"number", value}
	end
end

function notefield_prefs_speed_step_item()
	return {"item", notefield_prefs_config, "speed_step", "number", {translation_section= "notefield_options"}}
end

function notefield_prefs_speed_mod_item()
	return {
		"custom", {
			name= "speed_mod",
			arg= {config= notefield_prefs_config, path= "speed_mod"},
			adjust= function(direction, big, arg, pn)
				local data= arg.config:get_data(pn)
				local step= get_element_by_path(data, "speed_step")
				local amount= direction * step
				if big then amount= amount * 10 end
				local new_value= get_element_by_path(data, arg.path) + amount
				set_element_by_path(data, arg.path, new_value)
				arg.config:set_dirty(pn)
				nesty_menus.menu_message{
					category= "Config", config_name= arg.config.name,
					field_name= arg.path, value= new_value, pn= pn}
				return speed_mod_ret(data, new_value)
			end,
			value= function(arg, pn)
				local data= arg.config:get_data(pn)
				return speed_mod_ret(data, get_element_by_path(data, "speed_mod"))
			end,
			reset= function(arg, pn)
				local data= arg.config:get_data(pn)
				local new_value= 100
				set_element_by_path(data, arg.path, new_value)
				arg.config:set_dirty(pn)
				nesty_menus.menu_message{
					category= "Config", config_name= arg.config.name,
					field_name= arg.path, value= new_value, pn= pn}
				return speed_mod_ret(data, new_value)
			end,
			refresh= {
				category= "Config",
				config_name= notefield_prefs_config.name,
				field_name= "speed_type", match_pn= true,
			},
	}}
end

function notefield_prefs_speed_type_item()
	return {"item", notefield_prefs_config, "speed_type", "choice", {choices= notefield_speed_types, translation_section= "notefield_options"}}
end

function notefield_prefs_perspective_item()
	return {"item", notefield_prefs_config, "rotation_x", "name_value_pairs", {choices= {{"distant", -30}, {"overhead", 0}, {"hallway", 30}}}}
end
