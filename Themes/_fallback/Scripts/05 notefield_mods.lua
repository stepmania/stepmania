local notefield_mods= {
	effect= {
		beat= function(mag, field)
			field:all_columns_mod("get_note_pos_x", {name= "beat", {"*", 20 * mag, {"sin", {"+", math.pi/2, {"*", "y_offset", 1/15}}}, {"phase", {"repeat", "music_beat", 0, 2}, {default= {0, 0, 0, 0}, {0, .2, 5, -1}, {.8, 1, 5, 0}, {1, 1.2, -5, 1}, {1.8, 2, -5, 0}}}}})
		end,
		blink= function(mag, field)
			field:all_columns_mod("get_note_alpha", {name= "blink", {"sin", {"*", "music_second", 10 * mag}}})
		end,
		bumpy= function(mag, field)
			field:all_columns_mod("get_note_pos_y", {name= "bumpy", {"*", 40 * mag, {"sin", {"*", "y_offset", 1/16}}}})
		end,
		confusion= function(mag, field)
			field:all_columns_mod("get_note_rot_z", {name= "confusion", {"*", "music_beat", mag * 2 * math.pi}})
		end,
		dizzy= function(mag, field)
			field:all_columns_mod("get_note_rot_z", {name= "dizzy", {"*", "dist_beat", mag * 2 * math.pi}})
		end,
		drunk= function(mag, field)
			field:all_columns_mod("get_note_pos_x", {name= "drunk", {"*", mag * 32, {"cos", {"+", "music_second", {"*", "column", .2}, {"*", "y_offset", 10, 1/480}}}}})
		end,
		flip= function(mag, field)
			field:all_columns_mod("get_column_pos_x", {name= "flip", sum_type= "*", -1 * mag})
		end,
		roll= function(mag, field)
			field:all_columns_mod("get_note_rot_x", {name= "roll", {"*", "y_offset", .5 * (math.pi / 180)}})
		end,
		tiny= function(mag, field)
			local mod= {name= "tiny", sum_type= "*", {"exp", .5, mag}}
			field:all_columns_mod("get_note_zoom_x", mod)
			field:all_columns_mod("get_note_zoom_y", mod)
			field:all_columns_mod("get_note_zoom_z", mod)
			field:all_columns_mod("get_column_pos_x", {name= "tiny", sum_type= "*", {"min", 1, {"exp", .5, mag}}})
		end,
		tipsy= function(mag, field)
			field:all_columns_mod("get_column_pos_y", {name= "tipsy", {"*", mag, {"cos", {"+", {"*", "music_second", 1.2}, {"*", "column", 1.8}}}, 64 * .4}})
		end,
		twirl= function(mag, field)
			field:all_columns_mod("get_note_rot_y", {name= "twirl", {"*", "y_offset", .5 * mag * (math.pi / 180)}})
		end,
	},
	appearance= {
		flat= function(mag, field)
			field:all_columns_mod("get_quantization_multiplier", {name= "base_value", 1-mag})
		end,
		rainbow= function(mag, field)
			field:all_columns_mod("get_quantization_multiplier", {name= "base_value", 0})
			field:all_columns_mod("get_quantization_offset", {name= "rainbow", {"*", .125, "row_id"}})
		end,
		stealth= function(mag, field)
			field:all_columns_mod("get_note_alpha", {name= "base_value", 1-mag})
		end,
	},
	scrolls= {
		alternate= function(mag, field)
			for i, col in ipairs(field:get_columns()) do
				if i % 2 == 1 then
					col:get_reverse_scale():add_mod{name= "alternate", sum_type= "*", -1 * mag}
				end
			end
		end,
		centered= function(mag, field)
			field:all_columns_mod("get_center_percent", {name= "base_value", mag})
		end,
		cross= function(mag, field)
			local columns= field:get_columns()
			local first_cross= math.floor(#columns / 4)
			local last_cross= #columns - 1 - first_cross
			for i, col in ipairs(columns) do
				if (i-1) >= first_cross and (i-1) <= last_cross then
					col:get_reverse_scale():add_mod{name= "cross", sum_type= "*", -1 * mag}
				end
			end
		end,
		split= function(mag, field)
			local columns= field:get_columns()
			for i, col in ipairs(columns) do
				if i > #columns / 2 then
					col:get_reverse_scale():add_mod{name= "split", sum_type= "*", -1 * mag}
				end
			end
		end,
	},
}

-- todo:
-- boost, boomerang, brake, expand, wave
-- invert, tornado, xmode

local notefield_menu_choices= {
	appearance= {
		halved= {{"appearance.flat", .5}},
		doubled= {{"appearance.flat", -1}},
	},
}

local function choice_is_valid(choice)
	if type(choice) ~= "table" then return false end
	if #choice < 0 then return false end
	return true
end

function add_notefield_menu_choice(section, name, choice)
	assert(type(section) == "string" and #section > 0,
				 "First arg to add_notefield_menu_choice must be a section name.")
	assert(type(name) == "string" and #name > 0,
				 "Second arg to add_notefield_menu_choice must be a choice name.")
	assert(choice_is_valid(choice),
				 "Third arg to add_notefield_menu_choice must be a valid choice.")
	if not notefield_menu_choices[section] then
		notefield_menu_choices[section]= {}
	end
	notefield_menu_choices[section][name]= choice
end

foreach_ordered(
	notefield_mods, function(section_name, section)
		foreach_ordered(
			section, function(mod_name, func)
				add_notefield_menu_choice(section_name, mod_name, {{section_name.."."..mod_name, 1}})
		end)
end)

local player_mods= {
	[PLAYER_1]= {},
	[PLAYER_2]= {},
}

local profile_mods_config= create_lua_config{
	name= "notefield_mods", file= "notefield_mods.lua", default= {},
	match_depth= 0, use_alternate_config_prefix= "",
}
add_profile_load_callback(
	function(profile, dir, pn)
		if pn then
			profile_mods_config:load(pn)
			player_mods[pn]= DeepCopy(profile_mods_config:get_data(pn))
			rec_print_table(player_mods[pn])
			lua.ReportScriptError("Loaded mods for " .. pn)
		end
	end
)
add_profile_save_callback(standard_lua_config_profile_save(profile_mods_config))

function save_notefield_mods_to_profile(pn)
	profile_mods_config:set_data(pn, DeepCopy(player_mods[pn]))
	profile_mods_config:set_dirty(pn)
	profile_mods_config:save(pn)
end

function clear_notefield_mods_from_profile(pn)
	profile_mods_config:set_data(pn, {})
	profile_mods_config:set_dirty(pn)
	profile_mods_config:save(pn)
end

function clear_notefield_mods(pn)
	player_mods[pn]= {}
end

function add_notefield_mod(section, name, func)
	if not notefield_mods[section] then
		notefield_mods[section]= {}
	end
	notefield_mods[section][name]= func
end

local function get_multipart_mod_level(pn_mods, multi_mod)
	local level= false
	for i, part in ipairs(multi_mod) do
		if not pn_mods[part[1]] then
			return 0
		end
		local part_level= pn_mods[part[1]] / part[2]
		if not level then
			level= part_level
		else
			if math.abs(level - part_level) > 0.05 then
				return 0
			end
		end
	end
	return level
end

local function set_multipart_mod_level(pn, multi_mod, level)
	local pn_mods= player_mods[pn]
	if level == 0 then
		for i, part in ipairs(multi_mod) do
			pn_mods[part[1]]= nil
		end
	else
		for i, part in ipairs(multi_mod) do
			pn_mods[part[1]]= level * part[2]
		end
	end
end

local function notefield_mods_menu(with_save, no_sections, name, per_mod_func)
	local choices= {}
	choices[#choices+1]= {
		name= "notefield_mods_clear", translatable= true, execute= function(pn)
			clear_notefield_mods(pn)
		end,
	}
	if with_save then
		choices[#choices+1]= {
			name= "notefield_mods_profile_save", translatable= true,
			req_func= function(pn)
				return PROFILEMAN:IsPersistentProfile(pn)
			end,
			execute= function(pn)
				save_notefield_mods_to_profile(pn)
			end,
		}
		choices[#choices+1]= {
			name= "notefield_mods_profile_clear", translatable= true,
			req_func= function(pn)
				return PROFILEMAN:IsPersistentProfile(pn)
			end,
			execute= function(pn)
				clear_notefield_mods_from_profile(pn)
			end,
		}
	end
	local function add_section(name, section)
		local sub_choices= {}
		foreach_ordered(section, per_mod_func(name, sub_choices))
		if #sub_choices == 0 then return end
		if no_sections then
			for i, sub in ipairs(sub_choices) do
				choices[#choices+1]= sub
			end
		else
			choices[#choices+1]= nesty_options.submenu(name, sub_choices)
		end
	end
	foreach_ordered(notefield_menu_choices, add_section)
	return nesty_options.submenu(name, choices)
end

local function toggle_menu_per_mod(section_name, sub_choices)
	return function(name, mod)
		local full_name= section_name.."."..name
		sub_choices[#sub_choices+1]= {
			type= "bool", name= full_name, translatable= true, execute= function(pn)
				local pn_mods= player_mods[pn]
				local level= get_multipart_mod_level(pn_mods, mod)
				local new_val= 0
				if level ~= 1 then
					new_val= 1
				end
				set_multipart_mod_level(pn, mod, new_val)
				MESSAGEMAN:Broadcast("NotefieldModChanged", {name= full_name, value= new_val, pn= pn})
			end,
			value= function(pn)
				local pn_mods= player_mods[pn]
				local level= get_multipart_mod_level(pn_mods, mod)
				if level ~= 1 then
					return false
				else
					return true
				end
			end,
		}
	end
end

function get_notefield_mods_toggle_menu(with_save, no_sections)
	return notefield_mods_menu(with_save, no_sections, "notefield_toggle_mods", toggle_menu_per_mod)
end

local function value_menu_per_mod(section_name, sub_choices)
	return function(name, mod)
		local full_name= section_name.."."..name
		sub_choices[#sub_choices+1]= {
			name= full_name, translatable= true,
			menu= nesty_option_menus.adjustable_float, args= {
				name= full_name, min_scale= -2, scale= 0, max_scale= 0,
				initial_value= function(pn)
					return get_multipart_mod_level(player_mods[pn], mod)
				end,
				set= function(pn, value)
					set_multipart_mod_level(pn, mod, value)
					MESSAGEMAN:Broadcast("NotefieldModChanged", {name= full_name, value= value, pn= pn})
				end,
			},
			value= function(pn)
				return get_multipart_mod_level(player_mods[pn], mod)
			end,
		}
	end
end

function get_notefield_mods_value_menu(with_save, no_sections)
	return notefield_mods_menu(with_save, no_sections, "notefield_value_mods", value_menu_per_mod)
end

-- 1. Translating mod names has always been optional in stepmania.
-- 2. Seeing "foo.bar" in the menu and stuff will probably motivate themers
--    that make custom mods to translate them.
local function optional_trans(section, str)
	if THEME:HasString(section, str) then
		return THEME:GetString(section, str)
	end
	return str
end

function get_notefield_mods_as_string(pn)
	local ret= {}
	local function per_mod(name, value)
		if value == 1 then
			ret[#ret+1]= ("%s"):format(optional_trans("notefield_options", name))
		elseif value ~= 0 then
			ret[#ret+1]= ("%d%% %s"):format(value*100, optional_trans("notefield_options", name))
		end
	end
	foreach_ordered(player_mods[pn], per_mod)
	return table.concat(ret, ", ")
end

function get_player_options_as_string(pn, hide_fail)
	local ret= {}
	local ops= GAMESTATE:GetPlayerState(pn):get_player_options_no_defect("ModsLevel_Preferred")
	local life_type= ops:LifeSetting()
	if life_type == "LifeType_Bar" then
		local drain_type= ops:DrainSetting()
		if drain_type ~= "DrainType_Normal" then
			ret[#ret+1]= optional_trans("OptionNames", ToEnumShortString(drain_type))
		end
	elseif life_type == "LifeType_Battery" then
		local lives= ops:BatteryLives()
		if lives == 1 then
			ret[#ret+1]= ("%d %s"):format(lives, optional_trans("OptionNames", "Life"))
		else
			ret[#ret+1]= ("%d %s"):format(lives, optional_trans("OptionNames", "Lives"))
		end
	else
		ret[#ret+1]= optional_trans("OptionNames", "LifeTime")
	end
	if ops:Blind() ~= 0 then
		ret[#ret+1]= optional_trans("OptionNames", "Blind")
	end
	local cover= ops:Cover()
	if cover == 1 then
		ret[#ret+1]= optional_trans("OptionNames", "Cover")
	elseif cover ~= 0 then
		ret[#ret+1]= ("%d%% %s"):format(cover*100, optional_trans("OptionNames", "Cover"))
	end
	local bool_mods= {
		"Mirror", "Backwards", "Left", "Right", "Shuffle",
		"SoftShuffle", "SuperShuffle", "NoHolds", "NoRolls", "NoMines",
		"Little", "Wide", "Big", "Quick", "BMRize", "Skippy", "Mines",
		"AttackMines", "Echo", "Stomp", "Planted", "Floored", "Twister",
		"HoldRolls", "NoJumps", "NoHands", "NoLifts", "NoFakes", "NoQuads",
		"NoStretch", "MuteOnError",
	}
	for i, name in ipairs(bool_mods) do
		if ops[name](ops) then
			ret[#ret+1]= optional_trans("OptionNames", name)
		end
	end
	local fail= ops:FailSetting()
	if fail ~= "FailType_Immediate" and not hide_fail then
		ret[#ret+1]= optional_trans("OptionNames", fail)
	end
	return table.concat(ret, ", ")
end

function get_notefield_mods_with_player_options(pn, hide_fail)
	local nf_mods= get_notefield_mods_as_string(pn)
	local po_mods= get_player_options_as_string(pn, hide_fail)
	if #nf_mods > 0 then
		if #po_mods > 0 then
			return nf_mods .. ", " .. po_mods
		else
			return po_mods
		end
	else
		return po_mods
	end
end

function apply_notefield_mods(pn)
	local field= find_notefield_in_gameplay(SCREENMAN:GetTopScreen(), pn)
	if field then
		local function apply_mod(name, value)
			local func= get_element_by_path(notefield_mods, name)
			func(value, field)
		end
		foreach_ordered(player_mods[pn], apply_mod)
	end
end

function notefield_mods_actor()
	return Def.Actor{
		CurrentStepsP1ChangedMessageCommand= function(self, param)
			if not GAMESTATE:GetCurrentSteps(PLAYER_1) then return end
			self:queuecommand("delayed_p1_steps_change")
		end,
		CurrentStepsP2ChangedMessageCommand= function(self, param)
			if not GAMESTATE:GetCurrentSteps(PLAYER_2) then return end
			self:queuecommand("delayed_p2_steps_change")
		end,
		delayed_p1_steps_changeCommand= function(self)
			apply_notefield_mods(PLAYER_1)
		end,
		delayed_p2_steps_changeCommand= function(self)
			apply_notefield_mods(PLAYER_2)
		end,
	}
end
