function split_name(refstring)
	local parts= {}
	local cur_part_start= 1
	for i= 1, #refstring do
		local c= refstring:sub(i, i)
		if c == "." then
			parts[#parts+1]= refstring:sub(cur_part_start, i-1)
			cur_part_start= i+1
		end
	end
	if cur_part_start <= #refstring then
		parts[#parts+1]= refstring:sub(cur_part_start)
	end
	return parts
end

function get_element_by_path(container, path)
	local parts= split_name(path)
	local current= container
	for i= 1, #parts do
		current= current[parts[i]]
		if type(current) ~= "table" then return current end
	end
	return current
end

function set_element_by_path(container, path, value)
	local parts= split_name(path)
	local current= container
	for i= 1, #parts-1 do
		if not current[parts[i]] and value ~= nil then
			current[parts[i]]= {}
		end
		current= current[parts[i]]
	end
	current[parts[#parts]]= value
end

function string_in_table(str, tab)
	if not str or not tab then return false end
	for i, s in ipairs(tab) do
		if s == str then return i end
	end
	return false
end

local function force_table_elements_to_match_type(candidate, must_match, depth_remaining, exceptions)
	for k, v in pairs(candidate) do
		if not string_in_table(k, exceptions) then
			if type(must_match[k]) ~= type(v) then
				candidate[k]= nil
			elseif type(v) == "table" and depth_remaining ~= 0 then
				force_table_elements_to_match_type(v, must_match[k], depth_remaining-1, exceptions)
			end
		end
	end
	for k, v in pairs(must_match) do
		if type(candidate[k]) == "nil" then
			if type(v) == "table" then
				candidate[k]= DeepCopy(v)
			else
				candidate[k]= v
			end
		end
	end
end

local pn_slot_conversions= {}
for i, pn in ipairs(PlayerNumber) do
	if ProfileSlot[i] then
		pn_slot_conversions[pn]= ProfileSlot[i]
	end
end

local lua_config_mt= {
	__index= {
		init= function(self, params)
			self.name= params.name
			self.file= params.file
			self.default= params.default
			self.match_depth= params.match_depth
			self.dirty_table= {}
			self.data_set= {}
			self.exceptions= params.exceptions
			self.use_global_as_default= params.use_global_as_default
			self.use_alternate_config_prefix= params.use_alternate_config_prefix
			self.no_per_player= params.no_per_player
			self.is_lua_config= true

			-- Set the theme name part of the save path when the config object is
			-- created to avoid problems when a theme falls back on another.
			-- If get_filename() simply used the current theme name when called,
			-- then a config object loaded during script loading would load from a
			-- different prefix than where it saves to.
			local config_prefix= self.use_alternate_config_prefix or THEME:GetCurThemeName().."_config/"
			self.file_with_prefix= config_prefix..self.file
			return self
		end,
		sanitize_profile_slot= function(self, slot)
			if self.no_per_player then return "ProfileSlot_Invalid" end
			if pn_slot_conversions[slot] then return pn_slot_conversions[slot] end
			if not slot then return "ProfileSlot_Invalid" end
			return slot
		end,
		slot_to_prof_dir= function(self, slot, reason)
			local prof_dir= "Save/"
			slot= self:sanitize_profile_slot(slot)
			if slot == "ProfileSlot_Invalid" then return prof_dir end
			local checked= ""
			if slot:match("ProfileSlot") then
				checked= "using GetProfileDir"
				prof_dir= PROFILEMAN:GetProfileDir(slot)
			else
				checked= "using LocalProfileIDToDir"
				prof_dir= PROFILEMAN:LocalProfileIDToDir(slot)
			end
			if not prof_dir or prof_dir == "" then
				Trace("Could not fetch profile dir to " .. reason .. " for " .. tostring(slot) .. ".  " .. checked)
				if PREFSMAN:GetPreference("WarnOnNoProfile") then
					SCREENMAN:SystemMessage(THEME:GetString("Common", "NoProfileWarning"):format(ToEnumShortString(slot)))
				end
				return
			end
			return prof_dir
		end,
		apply_force= function(self, cand, slot)
			if self.match_depth and self.match_depth ~= 0 then
				force_table_elements_to_match_type(
					cand, self:get_default(slot), self.match_depth-1, self.exceptions)
			end
		end,
		get_default= function(self, slot)
			slot= self:sanitize_profile_slot(slot)
			if not self.use_global_as_default or slot == "ProfileSlot_Invalid" then
				return self.default
			end
			if not self.data_set["ProfileSlot_Invalid"] then
				self:load()
			end
			return self.data_set["ProfileSlot_Invalid"]
		end,
		load= function(self, slot)
			slot= self:sanitize_profile_slot(slot)
			local fname= self:get_filename(slot)
			if not fname or not FILEMAN:DoesFileExist(fname) then
				self.data_set[slot]= DeepCopy(self:get_default(slot))
			else
				local from_file= lua.load_config_lua(fname)
				if type(from_file) == "table" then
					self:apply_force(from_file, slot)
					self.data_set[slot]= from_file
				else
					self.data_set[slot]= DeepCopy(self:get_default(slot))
				end
			end
			return self.data_set[slot]
		end,
		get_data= function(self, slot)
			slot= self:sanitize_profile_slot(slot)
			if not self.data_set[slot] then
				self.data_set[slot]= DeepCopy(self:get_default(slot))
			end
			return self.data_set[slot]
		end,
		set_data= function(self, slot, data)
			slot= self:sanitize_profile_slot(slot)
			self.data_set[slot]= data
		end,
		set_dirty= function(self, slot)
			slot= self:sanitize_profile_slot(slot)
			self.dirty_table[slot]= true
		end,
		check_dirty= function(self, slot)
			slot= self:sanitize_profile_slot(slot)
			return self.dirty_table[slot]
		end,
		clear_slot= function(self, slot)
			slot= self:sanitize_profile_slot(slot)
			self.dirty_table[slot]= nil
			self.data_set[slot]= nil
		end,
		clear_all_slots= function(self)
			local slots_to_clear= {}
			for slot, data in pairs(self.data_set) do
				if slot ~= "ProfileSlot_Invalid" then
					slots_to_clear[#slots_to_clear+1]= slot
				end
			end
			for i, slot in ipairs(slots_to_clear) do
				self:clear_slot(slot)
			end
		end,
		get_filename= function(self, slot)
			slot= self:sanitize_profile_slot(slot)
			local prof_dir= self:slot_to_prof_dir(slot, "write " .. self.name)
			if not prof_dir then
				SCREENMAN:SystemMessage("Unable to get profile dir for " .. ToEnumShortString(slot))
				return
			end
			return prof_dir .. self.file_with_prefix
		end,
		save= function(self, slot)
			slot= self:sanitize_profile_slot(slot)
			if not self:check_dirty(slot) then return end
			local fname= self:get_filename(slot)
			if not fname then
				SCREENMAN:SystemMessage("Unable to save config.")
				return
			end
			lua.save_lua_table(fname, self.data_set[slot])
		end,
		save_all= function(self)
			for slot, data in pairs(self.data_set) do
				self:save(slot)
			end
		end,
		copy_data_from_old_instance= function(self, other)
			for slot, data in pairs(other.data_set) do
				self:apply_force(data, slot)
				self.data_set[slot]= data
			end
			for slot, dirty in pairs(other.dirty_table) do
				self.dirty_table[slot]= dirty
			end
		end,
}}

if not lua_config_registry then
	lua_config_registry= {}
end

function create_lua_config(params)
	assert(type(params.file) == "string", "create_lua_config requires a file field in the params.")
	if type(params.name) ~= "string" then params.name= params.file end
	assert(type(params.default) == "table", "creating a lua config without a default config table is nonsensical.")
	if params.use_alternate_config_prefix then
		assert(type(params.use_alternate_config_prefix) == "string", "create_lua_config: use_alternate_config_prefix must be nil or a string.")
	end
	if not params.match_depth then params.match_depth= -1 end
	local new_config_object= setmetatable({}, lua_config_mt):init(params)
	if lua_config_registry[params.name] then
		new_config_object:copy_data_from_old_instance(lua_config_registry[params.name])
	end
	lua_config_registry[params.name]= new_config_object
	return new_config_object
end

function standard_lua_config_profile_load(config)
	return function(profile, dir, pn)
		if pn then
			config:load(pn)
		end
	end
end
function standard_lua_config_profile_save(config)
	return function(profile, dir, pn)
		if pn then
			config:save(pn)
		end
	end
end

function add_standard_lua_config_save_load_hooks(config)
	add_profile_load_callback(standard_lua_config_profile_load(config))
	add_profile_save_callback(standard_lua_config_profile_save(config))
end

function clear_all_lua_config_slots()
	for name, config in pairs(lua_config_registry) do
		config:clear_all_slots()
	end
end
add_gamestate_reset_callback(clear_all_lua_config_slots)
