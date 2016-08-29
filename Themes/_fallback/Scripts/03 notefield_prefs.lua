local notefield_default_prefs= {
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
	local torad= math.pi / 180
	if prefs.speed_type then
		if prefs.speed_type == "maximum" then
			field:set_speed_mod(false, prefs.speed_mod, read_bpm)
		elseif prefs.speed_type == "constant" then
			field:set_speed_mod(true, prefs.speed_mod)
		else
			field:set_speed_mod(false, prefs.speed_mod)
		end
	end
	field:get_fov_mod():set_value(prefs.fov)
	field:get_vanish_x_mod():set_value(prefs.vanish_x)
	field:get_vanish_y_mod():set_value(prefs.vanish_y)
	field:get_trans_rot_x():set_value(prefs.rotation_x*torad)
	field:get_trans_rot_y():set_value(prefs.rotation_y*torad)
	field:get_trans_rot_z():set_value(prefs.rotation_z*torad)
	-- Use the y zoom to adjust the y offset to put the receptors in the same
	-- place.
	local adjusted_offset= prefs.yoffset / (prefs.zoom * prefs.zoom_y)
	for i, col in ipairs(field:get_columns()) do
		col:get_reverse_scale():set_value(prefs.reverse)
		col:get_reverse_offset_pixels():set_value(adjusted_offset)
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
	field:get_trans_zoom_x():set_value(prefs.zoom * prefs.zoom_x)
	field:get_trans_zoom_y():set_value(prefs.zoom * prefs.zoom_y)
	field:get_trans_zoom_z():set_value(prefs.zoom * prefs.zoom_z)
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
		poptions:XMod(prefs.speed_mod, 1000)
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
	}
end

function reset_needs_defective_field_for_all_players()
	for i, pn in ipairs{PLAYER_1, PLAYER_2} do
		GAMESTATE:GetPlayerState(pn):set_needs_defective_field(false)
	end
end

function advanced_notefield_prefs_menu()
	return nesty_options.submenu("advanced_notefield_config", {
		nesty_options.float_config_val(notefield_prefs_config, "hidden_offset", -1, 1, 2),
		nesty_options.float_config_val(notefield_prefs_config, "sudden_offset", -1, 1, 2),
		nesty_options.bool_config_val(notefield_prefs_config, "hidden"),
		nesty_options.bool_config_val(notefield_prefs_config, "sudden"),
		nesty_options.float_config_val(notefield_prefs_config, "fade_dist", -1, 1, 2),
		nesty_options.bool_config_val(notefield_prefs_config, "glow_during_fade"),
		nesty_options.float_config_val(notefield_prefs_config, "reverse", -2, 0, 0),
		nesty_options.float_config_val(notefield_prefs_config, "zoom", -2, -1, 1),
		nesty_options.float_config_val(notefield_prefs_config, "rotation_x", -1, 1, 2),
		nesty_options.float_config_val(notefield_prefs_config, "rotation_y", -1, 1, 2),
		nesty_options.float_config_val(notefield_prefs_config, "rotation_z", -1, 1, 2),
		-- Something tells me the notefield code still doesn't handle the vanish
		-- point right, so the vanish point options are disabled until I'm sure
		-- it's right. -Kyz
--		nesty_options.float_config_val(notefield_prefs_config, "vanish_x", -1, 1, 2),
--		nesty_options.float_config_val(notefield_prefs_config, "vanish_y", -1, 1, 2),
--		nesty_options.float_config_val(notefield_prefs_config, "fov", -1, 0, 1, 1, 179),
		nesty_options.float_config_val(notefield_prefs_config, "yoffset", -1, 1, 2),
		nesty_options.float_config_val(notefield_prefs_config, "zoom_x", -2, -1, 1),
		nesty_options.float_config_val(notefield_prefs_config, "zoom_y", -2, -1, 1),
		nesty_options.float_config_val(notefield_prefs_config, "zoom_z", -2, -1, 1),
	})
end

local function gen_speed_menu(pn)
	local prefs= notefield_prefs_config:get_data(pn)
	if prefs.speed_type == "multiple" then
		return nesty_options.float_config_val_args(notefield_prefs_config, "speed_mod", -2, -1, 1)
	else
		return nesty_options.float_config_val_args(notefield_prefs_config, "speed_mod", 0, 1, 3)
	end
end

function notefield_prefs_speed_mod_menu()
	return setmetatable({name= "speed_mod", menu= nesty_option_menus.adjustable_float,
	 translatable= true, args= gen_speed_menu, exec_args= true,
	 value= function(pn)
		 return notefield_prefs_config:get_data(pn).speed_mod
	 end}, mergable_table_mt)
end

function notefield_prefs_speed_type_menu()
	return setmetatable({name= "speed_type", menu= nesty_option_menus.enum_option,
	 translatable= true, value= function(pn)
		 return notefield_prefs_config:get_data(pn).speed_type
	 end,
	 args= {
		 name= "speed_type", enum= notefield_speed_types, fake_enum= true,
		 obj_get= function(pn) return notefield_prefs_config:get_data(pn) end,
		 get= function(pn, obj) return obj.speed_type end,
		 set= function(pn, obj, value)
			 if obj.speed_type == "multiple" and value ~= "multiple" then
				 obj.speed_mod= math.round(obj.speed_mod * 100)
			 elseif obj.speed_type ~= "multiple" and value == "multiple" then
				 obj.speed_mod= obj.speed_mod / 100
			 end
			 obj.speed_type= value
			 notefield_prefs_config:set_dirty(pn)
			 MESSAGEMAN:Broadcast("ConfigValueChanged", {
				config_name= notefield_prefs_config.name, field_name= "speed_type", value= value, pn= pn})
		 end,
	}}, mergable_table_mt)
end

local function trisign_of_num(num)
	if num < 0 then return -1 end
	if num > 0 then return 1 end
	return 0
end

-- Skew needs to shift towards the center of the screen.
local pn_skew_mult= {[PLAYER_1]= 1, [PLAYER_2]= -1}

local function perspective_entry(name, skew_mult, rot_mult)
	return setmetatable({
		name= name, translatable= true, type= "choice", execute= function(pn)
			local conf_data= notefield_prefs_config:get_data(pn)
			local old_rot= get_element_by_path(conf_data, "rotation_x")
			local old_skew= get_element_by_path(conf_data, "vanish_x")
			local new_rot= rot_mult * 30
			local new_skew= skew_mult * 160 * pn_skew_mult[pn]
			set_element_by_path(conf_data, "rotation_x", new_rot)
			set_element_by_path(conf_data, "vanish_x", new_skew)
			-- Adjust the y offset to make the receptors appear at the same final
			-- position on the screen.
			if new_rot < 0 then
				set_element_by_path(conf_data, "yoffset", 180)
			elseif new_rot > 0 then
				set_element_by_path(conf_data, "yoffset", 140)
			else
				set_element_by_path(conf_data, "yoffset", get_element_by_path(notefield_prefs_config:get_default(), "yoffset"))
			end
			MESSAGEMAN:Broadcast("ConfigValueChanged", {
				config_name= notefield_prefs_config.name, field_name= "rotation_x", value= new_rot, pn= pn})
		end,
		value= function(pn)
			local conf_data= notefield_prefs_config:get_data(pn)
			local old_rot= get_element_by_path(conf_data, "rotation_x")
			local old_skew= get_element_by_path(conf_data, "vanish_x")
			if trisign_of_num(old_rot) == trisign_of_num(rot_mult) and
			trisign_of_num(old_skew) == trisign_of_num(skew_mult) * pn_skew_mult[pn] then
				return true
			end
			return false
		end,
	}, mergable_table_mt)
end

function notefield_perspective_menu()
	return nesty_options.submenu("perspective", {
		perspective_entry("overhead", 0, 0),
		perspective_entry("distant", 0, -1),
		perspective_entry("hallway", 0, 1),
		perspective_entry("incoming", 1, -1),
		perspective_entry("space", 1, 1),
	})
end
