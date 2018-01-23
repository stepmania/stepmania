local notefield_mods= {
	effect= {
	},
	appearance= {
		all_note= function(mag)
			return {
				noteskin= {{random= "all"}},
				{name= "all_note", column= "all", target= "note_skin_id", {'*', 'row_id', mag}},
				{name= "all_note", column= "all", target= "layer_skin_id", {'*', 'music_beat', mag}},
			}
		end,
		flat= function(mag)
			return {{name= "flat", column= "all", target= "quantization_multiplier", sum_type= '*', 1-mag}}
		end,
		rainbow= function(mag)
			local phases= {}
			local quants= {0, 1/4, 1/3, 1/8, 1/16, 1/2, 1/6, 1/12}
			for i= 1, #quants do
				phases[i]= {i-1, i, 0, quants[i]}
			end
			return {
				{name= "rainbow", column= "all", target= "quantization_multiplier",
				 sum_type= '*', 0},
				{name= "rainbow", column= "all", target= "quantization_offset",
				 {"phase", {"repeat", "row_id", 0, #quants}, phases}},
			}
		end,
		stealth= function(mag)
			return {{name= "stealth", column= "all", 'stealth', sum_type= '*', mag}}
		end,
	},
	scrolls= {
		alternate= function(mag)
			return {{
					name= "alternate", column= function(num)
						local ret= {}
						for i= 1, num do
							if i % 2 == 1 then
								ret[#ret+1]= i
							end
						end
						return ret
					end,
					target= "reverse", sum_type= '*', -1*mag
			}}
		end,
		centered= function(mag)
			return {{name= "center", column= "all", target= "center", mag}}
		end,
		cross= function(mag)
			return {{
					name= "cross", column= function(num)
						local first_cross= math.floor(num / 4)
						local last_cross= num - 1 - first_cross
						local ret= {}
						for i= 1, num do
							if (i-1) >= first_cross and (i-1) <= last_cross then
								ret[#ret+1]= i
							end
						end
						return ret
					end,
					target= "reverse", sum_type= '*', -1 * mag
			}}
		end,
		split= function(mag)
			return {{
					name= "split", column= function(num)
						local half= num / 2
						local ret= {}
						for i= 1, num do
							if i > half then
								ret[#ret+1]= i
							end
						end
						return ret
					end,
					target= "reverse", sum_type= '*', -1 * mag
			}}
		end,
	},
}

for i, name in ipairs{
	"attenuate", "beat", "blink", "bounce", "bumpy", "confusion", 'digital',
	'dizzy', 'drunk', 'flip', 'invert', 'parabola', 'pulse', 'roll',
	'sawtooth', 'shrinklinear', 'shrinkmult', 'square', 'tiny',
	'tipsy', 'tornado', 'twirl', 'xmode', 'zigzag',
} do
	notefield_mods.effect[name]= function(mag)
		return {{name= name, column= "all", name, mag}}
	end
end

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
local player_clear_list= {
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
	local pn_clear= player_clear_list[pn]
	for name, level in pairs(player_mods[pn]) do
		pn_clear[name]= true
	end
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
	local pn_clear= player_clear_list[pn]
	if math.abs(level) < .001 then
		for i, part in ipairs(multi_mod) do
			pn_mods[part[1]]= nil
			pn_clear[part[1]]= true
		end
	else
		for i, part in ipairs(multi_mod) do
			pn_mods[part[1]]= level * part[2]
			pn_clear[part[1]]= nil
		end
	end
end

local function notefield_mods_menu(pn, with_save, no_sections, per_mod_func)
	local items= {
		{"action", "notefield_mods_clear",
		 function(big, arg, pn)
			 clear_notefield_mods(pn)
			 MESSAGEMAN:Broadcast("NoteFieldModChanged", {pn= pn})
		end},
	}
	if with_save and PROFILEMAN:IsPersistentProfile(pn) then
		items[#items+1]= {
			"action", "notefield_mods_profile_save",
			function(big, arg, pn) save_notefield_mods_to_profile(pn) end}
		items[#items+1]= {
			"action", "notefield_mods_profile_clear",
			function(big, arg, pn) clear_notefield_mods_from_profile(pn) end}
	end
	foreach_ordered(
		notefield_menu_choices, function(secname, section)
			local sub_items= {}
			foreach_ordered(section, per_mod_func(secname, sub_items))
			if #sub_items == 0 then return end
			if no_sections then
				for i, sub in ipairs(sub_items) do
					items[#items+1]= sub
				end
			else
				items[#items+1]= {"submenu", secname, sub_items}
			end
	end)
	return nesty_menus.add_close_item(nesty_menus.make_menu(items))
end

local function toggle_menu_per_mod(section_name, sub_items)
	return function(name, mod)
		local full_name= section_name.."."..name
		sub_items[#sub_items+1]= {
			"custom", {
				type_hint= {main= "bool"},
				refresh= {category= "NoteFieldMod", match_pn= true},
				name= full_name, func= function(big, arg, pn)
					local pn_mods= player_mods[pn]
					local level= get_multipart_mod_level(pn_mods, mod)
					local new_val= 0
					if level ~= 1 then
						new_val= 1
					end
					set_multipart_mod_level(pn, mod, new_val)
					nesty_menus.menu_message{category= "NoteFieldMod", pn= pn}
					MESSAGEMAN:Broadcast("NoteFieldModChanged", {name= full_name, value= new_val, pn= pn})
					if new_val == 1 then
						return {"boolean", true}
					else
						return {"boolean", false}
					end
				end,
				func_changes_value= true, value= function(arg, pn)
					local pn_mods= player_mods[pn]
					local level= get_multipart_mod_level(pn_mods, mod)
					if level == 1 then
						return {"boolean", true}
					else
						return {"boolean", false}
					end
		end}}
	end
end

function get_notefield_mods_toggle_menu(with_save, no_sections)
	return {
		"custom", {
			type_hint= {main= "submenu", sub= "note_mod"},
			name= "notefield_toggle_mods", func= function(big, arg, pn)
				local items= notefield_mods_menu(
					pn, with_save, no_sections, toggle_menu_per_mod)
				return "submenu", items
			end
	}}
end

local function value_menu_per_mod(section_name, sub_items)
	return function(name, mod)
		local full_name= section_name.."."..name
		sub_items[#sub_items+1]= {
			"custom", {
				type_hint= {main= "number", sub= "note_mod"},
				refresh= {category= "NoteFieldMod", match_pn= true},
				name= full_name, value= function(arg, pn)
					local value= get_multipart_mod_level(player_mods[pn], mod)
					return {"number", value}
				end,
				adjust= function(dir, big, arg, pn)
					local value= get_multipart_mod_level(player_mods[pn], mod)
					local step= .1 * dir
					if big then
						step= step * 10
					end
					value= value + step
					set_multipart_mod_level(pn, mod, value)
					nesty_menus.menu_message{category= "NoteFieldMod", pn= pn}
					MESSAGEMAN:Broadcast("NoteFieldModChanged", {name= full_name, value= value, pn= pn})
					return {"number", value}
				end,
		}}
	end
end

function get_notefield_mods_value_menu(with_save, no_sections)
	return {
		"custom", {
			type_hint= {main= "submenu", sub= "note_mod"},
			name= "notefield_value_mods", func= function(big, arg, pn)
				local items= notefield_mods_menu(
					pn, with_save, no_sections, value_menu_per_mod)
				return "submenu", items
			end
	}}
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
		local pn_clear= player_clear_list[pn]
		local field_clear= {}
		local column_clear= {}
		for name, clear in pairs(pn_clear) do
			local sub_func= get_element_by_path(notefield_mods, name)
			if type(sub_func) == "function" then
				local sub_mods= sub_func(0)
				for i, sub in ipairs(sub_mods) do
					local target= sub.target
					if not target and type(sub[1]) == "string" then
						local mod_entry= find_custom_mod(sub[1])
						if mod_entry then
							target= mod_entry.target
						end
					end
					local clear_info= {target= target, name= sub.name}
					if sub.column then
						column_clear[#column_clear+1]= clear_info
					else
						field_clear[#field_clear+1]= clear_info
					end
				end
			end
		end
		if #field_clear > 0 then
			field:remove_permanent_mods(field_clear)
		end
		if #column_clear > 0 then
			for i, col in ipairs(field:get_columns()) do
				col:remove_permanent_mods(column_clear)
			end
		end
		player_clear_list[pn]= {}
		local mods_table= {}
		local mods_skins= {}
		foreach_ordered(
			player_mods[pn], function(name, value)
				local sub_func= get_element_by_path(notefield_mods, name)
				if type(sub_func) == "function" then
					local sub_mods= sub_func(value)
					if type(sub_mods.noteskin) == "table" then
						for i, entry in ipairs(sub_mods.noteskin) do
							mods_skins[#mods_skins+1]= entry
						end
					elseif type(sub_mods.noteskin) == "string" then
						mods_skins[#mods_skins+1]= sub_mods.noteskin
					end
					for i, sub in ipairs(sub_mods) do
						mods_table[#mods_table+1]= sub
					end
				end
		end)
		if #mods_table > 0 or #mods_skins > 0 then
			mods_table.columns= field:get_num_columns()
			if #mods_skins > 0 then
				field:clear_to_base_skin()
				process_mod_skin_entries({[pn]= field}, mods_skins)
			end
			local organized= organize_notefield_mods_by_target(mods_table)
			field:set_per_column_permanent_mods(organized)
		end
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
		NoteFieldModChangedMessageCommand= function(self, param)
			local pn= param.pn
			apply_notefield_mods(pn)
		end,
		delayed_p1_steps_changeCommand= function(self)
			apply_notefield_mods(PLAYER_1)
		end,
		delayed_p2_steps_changeCommand= function(self)
			apply_notefield_mods(PLAYER_2)
		end,
	}
end
