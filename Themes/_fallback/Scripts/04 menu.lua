function table_range_insert(tab, pos, entries)
	for eid= 1, #entries do
		table.insert(tab, pos + eid - 1, entries[eid])
	end
end

function table_range_remove(tab, start, finish)
	local remove_amount= (finish - start) + 1
	local last_id= #tab
	for id= start, finish do
		if id + remove_amount > last_id then
			tab[id]= nil
		else
			tab[id]= tab[id+remove_amount]
		end
	end
	for id= finish+1, last_id do
		tab[id]= nil
	end
end

function rec_calc_actor_pos(actor)
	-- This doesn't handle zooming.
	if not actor then return 0, 0 end
	local x= actor:GetDestX()
	local y= actor:GetDestY()
	local wx, wy= 0, 0
	if actor.GetNumWrapperStates then
		local wrappers= actor:GetNumWrapperStates()
		for i= 1, wrappers do
			local nitori= actor:GetWrapperState(i)
			wx= wx + nitori:GetDestX()
			wy= wy + nitori:GetDestY()
		end
	end
	local px, py= rec_calc_actor_pos(actor:GetParent())
	return x+px+wx, y+py+wy
end

local function want_string(section, name)
	if THEME:HasString(section, name) then
		return THEME:GetString(section, name)
	end
	lua.ReportScriptError('String "'..section.."::"..name..'" is missing.')
	return name
end

local function add_part(self, part_name, clickables, handler, required_message, is_name, func_name)
	local actor_name= part_name .. "_actor"
	self[actor_name]= rec_find_child(self.container, part_name)
	if self[actor_name] then
		if clickables then
			clickables[#clickables+1]= {
				item= self, actor= self[actor_name], arg= part_name,
				is_name= is_name, func_name= func_name, handler= handler}
		end
	else
		if required_message then
			assert(false, required_message)
		end
	end
end

local function show_hide_part(self, part_name, show)
	local part= self[part_name]
	if part then
		if show then
			part:playcommand("Show")
		else
			part:playcommand("Hide")
		end
	end
end

local function click_it(it) if it then it:playcommand("Click") end end
local function ticket(it) if it then it:playcommand("Tick") end end

local function one_index_is_a_mistake(i, n)
	return ((i-1)%n)+1
end

local shared_functions= {
	set_active= function(self)
		self.active= true
		self.container:play_command_no_recurse("Active")
	end,
	set_inactive= function(self)
		self.active= false
		self.container:play_command_no_recurse("Inactive")
		self.info= nil
	end,
	scroll= function(self, num_items, scroll_type, from, to)
		self.container:play_command_no_recurse("Scroll", {num_items= num_items, scroll_type= scroll_type, from= from, to= to})
	end,
}

local item_controller_mt= {
	__index= {
		attach= function(self, actor, pn)
			-- return info for making parts clickable.
			-- item parts: item name, value, adjust up, adjust down
			self.container= actor
			self.pn= pn
			local clickables= {}
			add_part(self, "name", clickables, self.name_click, "Menu item must have name actor.", true, "func")
			add_part(self, "value", clickables, self.value_click, "Menu item must have value actor.", false, "adjust")
			clickables[#clickables].alt_func_name= "func"
			add_part(self, "adjust_up", clickables, self.adjust_up_click, nil, false, "adjust")
			add_part(self, "adjust_down", clickables, self.adjust_down_click, nil, false, "adjust")
			self.buttons= clickables
			self.container:playcommand("Playerize", self.pn)
		end,
		apply_translation_section= function(self, section)
			self.translation_section= section
		end,
		get_trans_section= function(self)
			return self.info.translation_section or self.translation_section or "OptionNames"
		end,
		set_info= function(self, info)
			local set_command= "SetItem"
			if self.info ~= nil then
				set_command= "RefreshItem"
			end
			self.info= info
			if info then
				self.container:play_command_no_recurse(set_command, info)
				local trans_name= info.name
				if not info.dont_translate_name then
					trans_name= want_string(self:get_trans_section(), info.name)
				end
				self.name_actor:playcommand("SetName", trans_name)
				self:refresh_value()
				show_hide_part(self, "adjust_up_actor", info.adjust)
				show_hide_part(self, "adjust_down_actor", info.adjust)
			else
				self.container:play_command_no_recurse("ClearItem")
			end
		end,
		refresh_value= function(self)
			if not self.info then return end
			local value_type= type(self.info.value)
			if value_type == "nil" then
				self:set_value(nil)
				self.container:play_command_no_recurse("SetTypeHint", self.info.type_hint)
			elseif value_type == "function" then
				local display_value= self.info.value(self.info.arg, self.pn)
				self:set_value(display_value)
			else
				self:set_value(self.info.value)
			end
		end,
		check_click= function(self, press_info)
			if not self.info then return end
			for i= 1, #self.buttons do
				local entry= self.buttons[i]
				if self.info[entry.func_name] or self.info[entry.alt_func_name] then
					if not (press_info.scroll_dir and entry.is_name) then
						if entry.actor:pos_in_clickable_area(press_info.mx, press_info.my) then
							if press_info.reset_button and self.info.reset then
								self:reset_value()
								return true
							else
								return entry.handler(self, press_info)
							end
						end
					end
				end
			end
		end,
		check_focus= function(self, mx, my)
			if not self.info then return end
			return self.container:pos_in_clickable_area(mx, my)
		end,
		name_click= function(self, press_info)
			if not self.info then return end
			return self:interact(press_info.big_any)
		end,
		value_click= function(self, press_info)
			if not self.info then return end
			if self.info.adjust then
				self:adjust(press_info.adjust_dir, press_info.big_repeat)
			else
				return self:interact(press_info.big_any)
			end
			return true
		end,
		adjust_up_click= function(self, press_info)
			self:adjust(press_info.scroll_dir or 1, press_info.big_any)
			return true
		end,
		adjust_down_click= function(self, press_info)
			self:adjust(press_info.scroll_dir or -1, press_info.big_any)
			return true
		end,
		set_value= function(self, v)
			if not self.info then return end
			if type(v) == "table" then
				local value= v[2]
				if v[1] == "string"
				and not self.info.dont_translate_value then
					v.untranslated= value
					v[2]= want_string(self:get_trans_section(), value)
				end
			end
			self.value_actor:playcommand("SetValue", v)
		end,
		interact= function(self, big)
			if not self.info then return end
			-- return submenu info if this is a submenu item.
			if self.info.func then
				self.name_actor:playcommand("Click")
				if self.info.func_changes_value then
					self:set_value(
						self.info.func(big, self.info.arg, self.pn))
					return true
				else
					local action, info, more_info=
						self.info.func(big, self.info.arg, self.pn)
					if action == nil then
						return true
					else
						return action, info, more_info
					end
				end
			end
		end,
		adjust= function(self, direction, big)
			if not self.info then return end
			if not self.info.adjust then return end
			if direction > 0 then
				click_it(self.adjust_up_actor)
			else
				click_it(self.adjust_down_actor)
			end
			self:set_value(
				self.info.adjust(direction, big, self.info.arg, self.pn))
		end,
		reset_value= function(self)
			if not self.info then return end
			if self.info.reset then
				self.container:play_command_no_recurse("Reset")
				self:set_value(self.info.reset(self.info.arg, self.pn))
			end
		end,
		gain_focus= function(self)
			if not self.info then return end
			if self.info.on_focus then
				self.info.on_focus(self.info.arg, self.pn)
			end
			self.container:play_command_no_recurse("GainFocus")
		end,
		lose_focus= function(self)
			if not self.info then return end
			if self.info.off_focus then
				self.info.off_focus(self.info.arg, self.pn)
			end
			self.container:play_command_no_recurse("LoseFocus")
		end,
}}

local function check_funcs(item, funcs)
	for i= 1, #funcs do
		local info= funcs[i]
		assert(type(item[info[1]]) == "function", info[2])
	end
end

local menu_scroller_mt= {
	__index= {
		attach= function(self, all_items)
			check_funcs(
				all_items[1], {
					{"set_info", "set_info= function(self, info)"},
					{"set_active", "set_active= function(self)"},
					{"set_inactive", "set_inactive= function(self)"},
					{"scroll", "scroll= function(self, num_items, scroll_type, from, to)"},
					{"gain_focus", "gain_focus= function(self)"},
					{"lose_focus", "lose_focus= function(self)"},
			})
			assert(#all_items > 1, "Must have more than one thing in menu_scroller.")
			-- scroll_off should call set_inactive internally, and scroll_on should
			-- call set_active internally.
			self.container= container
			local main_items= {}
			local spare_items= {}
			self.num_main= math.floor(#all_items * .5)
			for id= 1, self.num_main do
				-- intentionally zero indexed.
				main_items[id-1]= all_items[id]
			end
			local spare_start= self.num_main + 1
			self.num_spare= #all_items - self.num_main
			for id= spare_start, #all_items do
				spare_items[id-spare_start]= all_items[id]
				all_items[id]:set_inactive()
			end
			self.main_items= main_items
			self.spare_items= spare_items
			self.menu_offset= 0
			self.menu_pos= 0
			self.focus_pos= self.num_main * .5
			if self.num_main % 2 == 1 then
				self.focus_pos= math.floor(self.focus_pos)
			end
		end,
		max_menu_offset= function(self)
			if #self.info > self.num_main then
				-- max_offset + self.num_main == #self.info
				return #self.info - self.num_main
			else
				return 0
			end
		end,
		clamp_offset= function(self, offset)
			return clamp(offset, 0, self:max_menu_offset())
		end,
		set_info= function(self, info, pos)
			self.info= info
			for id= 0, self.num_spare-1 do
				local item= self.spare_items[id]
				item:set_info(nil)
			end
			if #info < 1 then
				for id= 0, self.num_main-1 do
					local item= self.main_items[id]
					item:set_info(nil)
				end
			else
				if not pos then pos= 1 end
				pos= clamp(pos, 1, #info)
				if self.wrapping then
					local info_start= pos - self.focus_pos
					for id= 0, self.num_main-1 do
						local item= self.main_items[id]
						local info_id= one_index_is_a_mistake(info_start+id, #info)
						item:set_info(info[info_id])
						item:scroll(self.num_main, "first", id, id)
						item:set_active()
					end
					self.menu_offset= (info_start-1) % #info
					self.menu_pos= self.focus_pos
				else
					self.menu_offset= self:clamp_offset(pos - self.focus_pos - 1)
					local num_to_set= math.min(self.num_main, #info)
					for id= 0, num_to_set-1 do
						local item= self.main_items[id]
						item:set_info(info[id+self.menu_offset+1])
						item:scroll(self.num_main, "first", id, id)
						item:set_active()
					end
					for id= num_to_set, self.num_main-1 do
						local item= self.main_items[id]
						item:set_info(nil)
						item:scroll(self.num_main, "first", id, id)
						item:set_inactive()
					end
					self.menu_pos= pos - self.menu_offset - 1
				end
			end
		end,
		non_wrapping_insert= function(self, entries, pos)
			table_range_insert(self.info, pos, entries)
			if pos > self.menu_offset + self.num_main then
				-- All after the visible area, nothing to do.
			elseif pos <= self.menu_offset then
				-- All before the visible area, just shift the menu offset.
				self.menu_offset= self.menu_offset + #entries
			else
				local spare_id= 0
				local shift_dist= #entries
				local insert_from= pos - self.menu_offset
				if pos <= self.menu_offset + self.menu_pos then
					shift_dist= -shift_dist
					self.menu_offset= self.menu_offset + #entries
				end
				for main_id= self.num_main-1, pos-self.menu_offset-1, -1 do
					local main= self.main_items[main_id]
					local spare= self.spare_items[spare_id]
					local shift_to= main_id + shift_dist
					local scroll_type= shift_to >= self.num_main and "off" or "normal"
					local info_id= self.menu_offset + main_id + 1
					main:scroll(self.num_main, scroll_type, main_id, shift_to)
					spare:set_info(self.info[info_id])
					spare:scroll(self.num_main, "on", insert_from-1, main_id)
					self.spare_items[spare_id]= main
					self.main_items[main_id]= spare
					spare_id= spare_id+1
				end
			end
		end,
		insert_info= function(self, entries, pos)
			if type(pos) ~= "number" then
				pos= #self.info+1
			end
			pos= clamp(pos, 1, #self.info+1)
			if type(entries) ~= "table" or #entries < 1 then
				entries= {entries}
			end
			if self.wrapping then
				if #self.info > self.num_main and self.menu_offset + self.num_main < #self.info then
					self:non_wrapping_insert(entries, pos)
				else
					local old_cursor_info_id= self.menu_offset + self.menu_pos + 1
					local old_offset= self.menu_offset
					table_range_insert(self.info, pos, entries)
					local new_size= #self.info
					local new_offset= (old_cursor_info_id - self.menu_pos) % new_size
					self.menu_offset= new_offset
					self:refresh_info()
				end
			else
				self:non_wrapping_insert(entries, pos)
			end
		end,
		non_wrapping_remove= function(self, start, finish)
			table_range_remove(self.info, start, finish)
			if start > self.menu_offset + self.num_main then
				-- After visible area, nothing to do.
			elseif finish <= self.menu_offset then
				-- Before the visible area, just shift the menu offset.
				self.menu_offset= self.menu_offset - ((finish - start) + 1)
			else
				if self.menu_offset >= #self.info then
					self.menu_offset= self:clamp_offset(self.menu_offset)
					self:shift_all_items(1)
					if self.menu_offset + self.menu_pos >= #self.info then
						self.menu_pos= math.max(0, #self.info - self.menu_offset - 1)
					end
				else
					local old_offset= self.menu_offset
					self.menu_offset= self:clamp_offset(self.menu_offset)
					-- TODO: Scroll items around nicely.
					for main_id= 0, self.num_main-1 do
						local main= self.main_items[main_id]
						local info_id= self:item_id_to_info_id(main_id)
						local info= self.info[info_id]
						if info ~= main.info then
							main:set_info(info)
						end
					end
				end
			end
			if self:item_id_to_info_id(self.menu_pos) > #self.info then
				self.menu_pos= math.max(0, #self.info - self.menu_offset - 1)
			end
		end,
		remove_info= function(self, start, finish)
			if type(start) ~= "number" then
				start= #self.info
			end
			if type(finish) ~= "number" then
				finish= start
			end
			start= clamp(start, 1, #self.info)
			finish= clamp(finish, 1, #self.info)
			if finish < start then
				finish, start= start, finish
			end
			if self.wrapping then
				if #self.info > self.num_main and self.menu_offset + self.num_main < #self.info then
					self:non_wrapping_remove(start, finish)
				else
					local old_cursor_info_id= self.menu_offset + self.menu_pos + 1
					local old_offset= self.menu_offset
					table_range_remove(self.info, start, finish)
					local new_size= #self.info
					local new_offset= (old_cursor_info_id - self.menu_pos) % new_size
					self.menu_offset= new_offset
					self:refresh_info()
				end
			else
				if finish >= #self.info and self.menu_offset > 0 then
					local rma= (finish - start) + 1
					local sma= math.min(rma, self.menu_offset)
					self:scroll_items(-sma)
				end
				self:non_wrapping_remove(start, finish)
			end
		end,
		refresh_info= function(self)
			if #self.info < 1 then
				for id= 0, self.num_main-1 do
					self.main_items[id]:set_info(nil)
				end
				return
			end
			for id= 0, self.num_main-1 do
				local info_id= self:item_id_to_info_id(id)
				local info= self.info[info_id]
				local item= self.main_items[id]
				if info ~= item.info then
					item:set_info(info)
				end
			end
		end,
		shift_all_items= function(self, dir)
			local dist= dir * self.num_main
			for id= 0, self.num_main-1 do
				local main= self.main_items[id]
				local spare= self.spare_items[id]
				main:scroll(self.num_main, "off", id, id + dist)
				local info_id= self:item_id_to_info_id(id)
				spare:set_info(self.info[info_id])
				spare:scroll(self.num_main, "on", id - dist, id)
				self.main_items[id]= spare
				self.spare_items[id]= main
			end
		end,
		left_shift_items= function(self, amount)
			if amount >= self.num_main then
				self:shift_all_items(-1)
				return
			end
			local to_spare= {}
			for id= 0, self.num_main-1 do
				local item= self.main_items[id]
				local to= id-amount
				if to < 0 then
					item:scroll(self.num_main, "off", id, to)
					to_spare[#to_spare+1]= item
				else
					item:scroll(self.num_main, "normal", id, to)
					self.main_items[to]= item
				end
			end
			for id= 0, amount-1 do
				local spare= self.spare_items[id]
				local to= self.num_main - amount + id
				local from= to + amount
				local info_id= self:item_id_to_info_id(to)
				spare:set_info(self.info[info_id])
				spare:scroll(self.num_main, "on", from, to)
				self.main_items[to]= spare
				self.spare_items[id]= to_spare[id+1]
			end
		end,
		right_shift_items= function(self, amount)
			if amount >= self.num_main then
				self:shift_all_items(1)
				return
			end
			local to_spare= {}
			for id= self.num_main-1, 0, -1 do
				local item= self.main_items[id]
				local to= id+amount
				if to >= self.num_main then
					item:scroll(self.num_main, "off", id, to)
					to_spare[#to_spare+1]= item
				else
					item:scroll(self.num_main, "normal", id, to)
					self.main_items[to]= item
				end
			end
			for id= 0, amount-1 do
				local spare= self.spare_items[id]
				local to= id
				local from= to - amount
				local info_id= self:item_id_to_info_id(to)
				spare:set_info(self.info[info_id])
				spare:scroll(self.num_main, "on", from, to)
				self.main_items[to]= spare
				self.spare_items[id]= to_spare[id+1]
			end
		end,
		shift_items= function(self, amount)
			if amount > 0 then
				self:left_shift_items(amount)
			else
				self:right_shift_items(-amount)
			end
		end,
		cursor_up= function(self)
			self.main_items[self.menu_pos]:lose_focus()
			if self.wrapping then
				self.menu_offset= (self.menu_offset - 1) % #self.info
				self:right_shift_items(1)
			else
				if self.menu_offset > 0 then
					if self.menu_pos > self.focus_pos then
						self.menu_pos= self.menu_pos - 1
					else
						self.menu_offset= self.menu_offset - 1
						self:right_shift_items(1)
					end
				else
					if self.menu_pos > 0 then
						self.menu_pos= self.menu_pos - 1
					else
						if #self.info > self.num_main then
							self:jump(#self.info)
						else
							self.menu_pos= #self.info-1
						end
					end
				end
			end
			self.main_items[self.menu_pos]:gain_focus()
		end,
		cursor_down= function(self)
			self.main_items[self.menu_pos]:lose_focus()
			if self.wrapping then
				self.menu_offset= (self.menu_offset + 1) % #self.info
				self:left_shift_items(1)
			else
				if #self.info > self.num_main then
					if self.menu_pos < self.focus_pos then
						self.menu_pos= self.menu_pos + 1
					else
						if self.menu_offset < self:max_menu_offset() then
							self.menu_offset= self.menu_offset + 1
							self:left_shift_items(1)
						else
							if self.menu_pos < self.num_main-1 then
								self.menu_pos= self.menu_pos + 1
							else
								self:jump(1)
							end
						end
					end
				else
					self.menu_pos= (self.menu_pos + 1) % #self.info
				end
			end
			self.main_items[self.menu_pos]:gain_focus()
		end,
		scroll_items= function(self, dir)
			local new_offset= self:clamp_offset(self.menu_offset + dir)
			if new_offset ~= self.menu_offset then
				self.main_items[self.menu_pos]:lose_focus()
				self.menu_offset= new_offset
				self:shift_items(dir)
				self.main_items[self.menu_pos]:gain_focus()
			end
		end,
		page= function(self, dir)
			self.main_items[self.menu_pos]:lose_focus()
			local new_offset= self.menu_offset + (self.num_main * dir)
			if self.wrapping then
				self.menu_offset= new_offset % #self.info
				self:shift_all_items(dir)
			else
				new_offset= self:clamp_offset(new_offset)
				local shift= new_offset - self.menu_offset
				if shift ~= 0 then
					self.menu_offset= new_offset
					self:shift_items(shift)
				else
					if dir < 0 then
						self.menu_pos= 0
					else
						if #self.info > self.num_main then
							self.menu_pos= self.num_main-1
						else
							self.menu_pos= #self.info-1
						end
					end
				end
			end
			self.main_items[self.menu_pos]:gain_focus()
		end,
		page_up= function(self)
			self:page(-1)
		end,
		page_down= function(self)
			self:page(1)
		end,
		jump= function(self, pos)
			pos= one_index_is_a_mistake(pos, #self.info)
			self.main_items[self.menu_pos]:lose_focus()
			if self.num_main < #self.info then
				-- Convert to zero indexed positions.
				local cursor_pos= self:get_cursor_info_pos() - 1
				pos= pos - 1
				local new_offset= self.menu_offset
				if self.wrapping then
					new_offset= (pos - self.focus_pos) % #self.info
					self.menu_pos= self.focus_pos
				else
					new_offset= self:clamp_offset(pos - self.focus_pos)
					-- menu_offset + menu_pos == pos
					self.menu_pos= pos - new_offset
				end
				if new_offset ~= self.menu_offset then
					local dist= new_offset - self.menu_offset
					self.menu_offset= new_offset
					self:shift_items(dist)
				end
			else
				self.menu_pos= pos-1
			end
			self.main_items[self.menu_pos]:gain_focus()
		end,
		change_focus= function(self, id)
			if id < 0 or id >= self.num_main then return end
			self.main_items[self.menu_pos]:lose_focus()
			self.menu_pos= id
			self.main_items[self.menu_pos]:gain_focus()
		end,
		get_cursor_item= function(self)
			return self.main_items[self.menu_pos]
		end,
		get_cursor_info_pos= function(self)
			if self.wrapping then
				return ((self.menu_offset + self.menu_pos) % #self.info) + 1
			else
				return self.menu_offset + self.menu_pos + 1
			end
		end,
		item_id_to_info_id= function(self, id)
			if self.wrapping then
				return ((self.menu_offset + id) % #self.info) + 1
			else
				return self.menu_offset + id + 1
			end
		end,
		hide= function(self)
			self.container:play_command_no_recurse("Hide")
		end,
		show= function(self)
			self.container:play_command_no_recurse("Show")
		end,
}}

local display_controller_mt= {
	__index= {
		attach= function(self, container, pn)
			-- create item_controllers
			-- attach item_controller_mts
			-- return clickable info from items
			-- return focusable info for display so it can react to mouse moving
			self.container= container
			local items= rec_find_child(container, "item")
			assert(items, "Menu display must have item actors.")
			assert(#items > 0, "Name all the menu item actors 'item', and have more than one.")
			local item_controllers= {}
			for id= 1, #items do
				local curr_item= items[id]
				local controller= setmetatable({}, item_controller_mt)
				local success, message= pcall(controller.attach, controller, curr_item, pn)
				assert(success, "Problem in item " .. id .. ": " .. tostring(message))
				for c= 1, #controller.buttons do
					controller.buttons[c].display= self
				end
				item_controllers[#item_controllers+1]= controller
			end
			self.mouse_scroll_area= rec_find_child(container, "mouse_scroll_area")
			self.scroller= setmetatable({}, menu_scroller_mt)
			self.scroller:attach(item_controllers)
			self.container:playcommand("Playerize", self.pn)
		end,
		apply_translation_section= function(self, section)
			self:set_translation_section(self.scroller.main_items, self.scroller.num_main, section)
			self:set_translation_section(self.scroller.spare_items, self.scroller.num_spare, section)
		end,
		set_translation_section= function(self, group, group_size, section)
			for id= 0, group_size-1 do
				group[id]:apply_translation_section(section)
			end
		end,
		set_info= function(self, info)
			-- info has menu info, and info for each item.
			if self.info then
				if info then
					self.container:play_command_no_recurse("RefreshSubmenu", info)
				else
					self.container:play_command_no_recurse("CloseSubmenu")
				end
			else
				if info then
					self.container:play_command_no_recurse("OpenSubmenu", info)
				end
			end
			self.info= info
			if info then
				self.scroller:set_info(info, info.remembered_pos)
			else
				self.scroller:set_info({}, 1)
			end
		end,
		refresh_info= function(self, info)
			info.remembered_pos= info.remembered_pos or self.scroller:get_cursor_info_pos()
			self:set_info(info)
		end,
		check_click= function(self, press_info)
			if not self.info or #self.info < 1 then return end
			for id= 0, self.scroller.num_main-1 do
				local action, info, more_info= self.scroller.main_items[id]:check_click(press_info)
				if action then
					return action, info, more_info
				end
			end
		end,
		check_focus= function(self, mx, my)
			return self.container:pos_in_clickable_area(mx, my)
		end,
		update_sub_focus= function(self, getting_focus, mx, my)
			for id= 0, self.scroller.num_main-1 do
				if self.scroller.main_items[id]:check_focus(mx, my) then
					if getting_focus or id ~= self.scroller.menu_pos then
						self.scroller:change_focus(id)
						return true
					end
				end
			end
		end,
		cursor_move= function(self, dir)
			if dir < 0 then
				self.scroller:cursor_up()
			else
				self.scroller:cursor_down()
			end
		end,
		cursor_up= function(self)
			self.scroller:cursor_up()
		end,
		cursor_down= function(self)
			self.scroller:cursor_down()
		end,
		scroll_items= function(self, dir)
			self.scroller:scroll_items(dir)
		end,
		page_up= function(self)
			self.scroller:page_up()
		end,
		page_down= function(self)
			self.scroller:page_down()
		end,
		jump= function(self, pos)
			self.scroller:jump(pos)
		end,
		cursor_to_top= function(self)
			self.scroller:jump(1)
		end,
		cursor_to_bottom= function(self)
			self.scroller:jump(#self.info)
		end,
		get_cursor_item= function(self)
			return self.scroller:get_cursor_item()
		end,
		get_cursor_info_pos= function(self)
			return self.scroller:get_cursor_info_pos()
		end,
		gain_focus= function(self)
			self.container:play_command_no_recurse("GainFocus")
		end,
		lose_focus= function(self)
			self.container:play_command_no_recurse("LoseFocus")
		end,
}}

for name, func in pairs(shared_functions) do
	item_controller_mt.__index[name]= func
	display_controller_mt.__index[name]= func
end

local menu_convert= {
	Start= "start",
	Select= "select",
	Back= "back",
	MenuLeft= "left",
	MenuRight= "right",
	MenuUp= "up",
	MenuDown= "down",
}
local page_convert= {
	DeviceButton_pgdn= "page_down",
	DeviceButton_pgup= "page_up",
	DeviceButton_home= "jump_top",
	DeviceButton_end= "jump_bottom",
}
menu_controller_mt= {
	__index= {
		input_modes= {
			two_direction= true, -- LR + Start
			two_direction_with_select= true, -- LR + Start + Select
			four_direction= true, -- LRUD + Start + Select
		},
		init= function(self, args)
			self:attach(args.actor, args.pn)
			self:apply_translation_section(args.translation_section)
			self:set_input_mode(args.input_mode, args.repeats_to_big, args.select_goes_to_top)
			self:set_info(args.data, args.custom_menu)
		end,
		apply_translation_section= function(self, section)
			self:set_translation_section(self.scroller.main_items, self.scroller.num_main, section)
			self:set_translation_section(self.scroller.spare_items, self.scroller.num_spare, section)
		end,
		set_translation_section= function(self, group, group_size, section)
			for id= 0, group_size-1 do
				group[id]:apply_translation_section(section)
			end
		end,
		attach= function(self, container, pn)
			-- attach display_controllers to displays.
			-- store clickable info from menu items.
			if type(pn) ~= "string" then pn= nil end
			self.container= container
			self.pn= pn
			local displays= rec_find_child(container, "display")
			assert(displays, "Menu must have display actor.")
			self.cursor= rec_find_child(container, "cursor")
			local display_controllers= {}
			for id= 1, #displays do
				local curr_display= displays[id]
				local controller= setmetatable({}, display_controller_mt)
				local success, message= pcall(controller.attach, controller, curr_display, pn)
				assert(success, "Problem in display " .. id .. ": " .. tostring(message))
				display_controllers[#display_controllers+1]= controller
			end
			self.scroller= setmetatable({}, menu_scroller_mt)
			self.scroller:attach(display_controllers)
			self.input_mode= "four_direction"
			self.container:playcommand("Playerize", self.pn)
			self.menu_stack= {}
			self.scroller:set_info(self.menu_stack, 1)
		end,
		set_input_mode= function(
				self, mode, repeats_to_big, select_goes_to_top)
			-- All modes accept paging keys.
			if self.input_modes[mode] then
				self.input_mode= mode
				self.in_adjust_mode= false
			end
			if type(repeats_to_big) ~= "number" then
				self.repeats_to_big= 10
			else
				self.repeats_to_big= repeats_to_big
			end
			if select_goes_to_top ~= nil then
				self.select_goes_to_top= select_goes_to_top
			else
				self.select_goes_to_top= true
			end
			self.repeat_counts= {}
		end,
		set_info= function(self, info, custom)
			if custom then
				self.info= info
			else
				self.info= nesty_menus.make_menu(info)
			end
			self.menu_stack= {}
			self.scroller:set_info(self.menu_stack, 1)
		end,
		open_menu= function(self)
			if not self.info then return end
			self.container:play_command_no_recurse("OpenMenu")
			self:push_menu(self.info)
			self:update_cursor()
		end,
		close_menu= function(self)
			if not self.info then return end
			for id= 0, self.scroller.num_main-1 do
				local disp= self.scroller.main_items[id]
				if disp.info then
					disp:set_info(nil)
				end
			end
			for id= #self.menu_stack, 1, -1 do
				local entry= self.menu_stack[id]
				if entry.on_close then
					entry.on_close(entry.on_close_arg, self.pn)
				end
			end
			self.menu_stack= {}
			self.scroller:set_info(self.menu_stack, 1)
			self.container:play_command_no_recurse("CloseMenu")
		end,
		input= function(self, event)
			if #self.menu_stack < 1 then return end
			local pressie= ToEnumShortString(event.type):lower()
			if event.DeviceInput.is_mouse then
				return self:mouse_click(
					INPUTFILTER:GetMouseX(), INPUTFILTER:GetMouseY(),
					ToEnumShortString(event.DeviceInput.button), pressie)
			else
				local button= ""
				local low_level_button= event.DeviceInput.button
				if event.GameButton and event.GameButton ~= "" then
					button= event.GameButton
				end
				button= page_convert[low_level_button] or menu_convert[button]
				if button then
					return self:menu_button(button, pressie)
				end
			end
		end,
		handle_repeats= function(self, button, press_type)
			if not self.info then return end
			if press_type == "firstpress" then
				self.repeat_counts[button]= 0
			elseif press_type == "repeat" then
				self.repeat_counts[button]= (self.repeat_counts[button] or 0) + 1
			elseif press_type == "release" then
				self.repeat_counts[button]= 0
				return
			end
			return true, self.repeat_counts[button] >= self.repeats_to_big
		end,
		mouse_click= function(self, mx, my, button, press_type)
			local process, big_repeat= self:handle_repeats(button, press_type)
			if not process then return end
			local wheel_to_dir= {
				["mousewheel up"]= 1,
				["mousewheel down"]= -1,
			}
			local button_to_dir= {
				["mousewheel up"]= 1,
				["mousewheel down"]= -1,
				["left mouse button"]= 1,
				["right mouse button"]= -1,
			}
			local press_info= {
				mx= mx, my= my,
				scroll_dir= wheel_to_dir[button],
				adjust_dir= button_to_dir[button],
				big_button= button == "right mouse button",
				big_repeat= big_repeat,
				reset_button= button == "middle mouse button",
			}
			press_info.big_any= press_info.big_button or press_info.big_repeat
			for id= 0, self.scroller.num_main-1 do
				local disp= self.scroller.main_items[id]
				if press_info.scroll_dir then
					if disp.mouse_scroll_area then
						if disp.mouse_scroll_area:pos_in_clickable_area(mx, my) then
							disp:scroll_items(-press_info.scroll_dir)
							self:update_cursor()
							return nil, press_info.scroll_dir > 0 and "cursor_up" or "cursor_down"
						end
					end
				end
				local action, info, more_info= disp:check_click(press_info)
				if action then
					if type(action) == "string" then
						local pop_to= self.scroller:item_id_to_info_id(id)
						self:pop_menus_to(pop_to)
						return self:handle_menu_action(action, info, more_info)
					else
						return #self.menu_stack, "action"
					end
				end
			end
		end,
		menu_button= function(self, button, press_type)
			local process, big= self:handle_repeats(button, press_type)
			if not process then return end
			local active_display= self.scroller:get_cursor_item()
			local function sound_ret(name) return nil, name end
			local global_handlers= {
				page_down= function()
					active_display:page_down()
					self:update_cursor()
					return sound_ret("page_down")
				end,
				page_up= function()
					active_display:page_down()
					self:update_cursor()
					return sound_ret("page_up")
				end,
				jump_top= function()
					active_display:cursor_to_top()
					self:update_cursor()
					return sound_ret("jump_top")
				end,
				jump_bottom= function()
					active_display:cursor_to_bottom()
					self:update_cursor()
					return sound_ret("jump_bottom")
				end,
				back= function()
					return self:handle_menu_action("close", 1)
				end,
			}
			local global= global_handlers[button]
			if global then return global() end
			local item= active_display:get_cursor_item()
			local sub_handlers= {
				adjust= function(dir, sound)
					item:adjust(dir, big)
					return sound_ret(sound)
				end,
				interact= function()
					return self:handle_menu_action(item:interact(big))
				end,
				adjust_or_interact= function(dir, sound)
					if item.info.adjust then
						item:adjust(dir, big)
						return sound_ret(sound)
					elseif item.info.func then
						return self:handle_menu_action(item:interact(big))
					end
				end,
				cursor_move= function(dir, sound)
					active_display:cursor_move(dir)
					self:update_cursor()
					return sound_ret(sound)
				end,
				two_dir_interact= function()
					if item.info.func then
						return self:handle_menu_action(item:interact(big))
					elseif item.info.adjust then
						if self.in_adjust_mode then
							self.in_adjust_mode= false
							if self.cursor then
								self.cursor:playcommand("NormalMode")
							end
							return sound_ret("adjust_mode_off")
						else
							self.in_adjust_mode= true
							if self.cursor then
								self.cursor:playcommand("AdjustMode")
							end
							return sound_ret("adjust_mode_on")
						end
					end
				end,
				select_jump= function()
					if self.select_goes_to_top then
						active_display:cursor_to_top()
						self:update_cursor()
						return sound_ret("jump_top")
					else
						active_display:cursor_to_bottom()
						self:update_cursor()
						return sound_ret("jump_bottom")
					end
				end,
			}
			sub_handlers.two_dir_adjust= function(dir, adj_sound, curs_sound)
					if self.in_adjust_mode then
						return sub_handlers.adjust(dir, adj_sound)
					else
						return sub_handlers.cursor_move(dir, curs_sound)
					end
			end
			local button_to_sub= {
				two_direction= {
					left= {"two_dir_adjust", -1, "adjust_down", "cursor_up"},
					right= {"two_dir_adjust", 1, "adjust_up", "cursor_down"},
					start= {"two_dir_interact"},
				},
				two_direction_with_select= {
					left= {"adjust_or_interact", -1, "adjust_down"},
					right= {"adjust_or_interact", 1, "adjust_up"},
					start= {"cursor_move", 1, "cursor_down"},
					select= {"cursor_move", -1, "cursor_up"},
				},
				four_direction= {
					left= {"adjust_or_interact", -1, "adjust_down"},
					right= {"adjust_or_interact", 1, "adjust_up"},
					up= {"cursor_move", -1, "cursor_up"},
					down= {"cursor_move", 1, "cursor_down"},
					start= {"interact"},
					select= {"select_jump"},
				},
			}
			local for_mode= button_to_sub[self.input_mode]
			if for_mode then
				local entry= for_mode[button]
				if entry then
					return sub_handlers[entry[1]](entry[2], entry[3], entry[4])
				end
			end
		end,
		get_cursor_item= function(self)
			local disp= self.scroller:get_cursor_item()
			if disp then
				return disp:get_cursor_item()
			end
		end,
		handle_menu_action= function(self, action, info, extra)
			if not action then return end
			local active_display= self.scroller:get_cursor_item()
			local action_sound= false
			local function handle_refresh(items)
				self.menu_stack[#self.menu_stack]= items
				active_display:refresh_info(items)
				self:update_cursor()
			end
			if action == "refresh" then
				if type(info) == "table" then
					handle_refresh(info)
					action_sound= "refresh"
				end
			elseif action == "submenu" then
				if type(info) == "table" then
					self:push_menu(info)
					action_sound= "submenu"
				end
			elseif action == "close" then
				if type(info) ~= "number" then
					info= 1
				end
				if type(extra) == "table" then
					if info < 0 then
						info= #self.menu_stack-1
					end
					for i= 1, info do
						self:pop_menu()
					end
					active_display= self.scroller:get_cursor_item()
					handle_refresh(extra)
					action_sound= "close_submenu"
				else
					if info < 0 or info >= #self.menu_stack then
						self:close_menu()
						action_sound= "close_menu"
					else
						for i= 1, info do
							self:pop_menu()
						end
						action_sound= "close_submenu"
					end
				end
			end
			return #self.menu_stack, action_sound
		end,
		push_menu= function(self, info)
			if info.on_open then
				info.on_open(info.on_open_arg, self.pn)
			end
			local old_top= self.menu_stack[#self.menu_stack]
			if old_top then
				old_top.remembered_pos= self.scroller:get_cursor_item():get_cursor_info_pos()
			end
			self.scroller:insert_info({info})
			self.scroller:jump(#self.menu_stack)
			self:update_cursor()
		end,
		pop_menu= function(self)
			if #self.menu_stack <= 1 then return end
			local pop_id= #self.menu_stack
			local being_popped= self.menu_stack[pop_id]
			local remembered_pos= being_popped.remembered_pos
			if being_popped.on_close then
				being_popped.on_close(being_popped.on_close_arg, remembered_pos, self.pn)
			end
			self.scroller:remove_info(pop_id)
			self:update_cursor()
		end,
		pop_menus_to= function(self, id)
			while #self.menu_stack > id do
				self:pop_menu()
			end
		end,
		update_focus= function(self, mx, my)
			if #self.menu_stack < 1 then return end
			local old_disp_id= self.scroller.menu_pos
			local old_item_id= self.scroller:get_cursor_item().scroller.menu_pos
			for id= 0, self.scroller.num_main-1 do
				local disp= self.scroller.main_items[id]
				if disp.info then
					if disp:check_focus(mx, my) then
						local getting_focus= id ~= old_disp_id
						if disp:update_sub_focus(getting_focus, mx, my) then
							if getting_focus then
								self.scroller:change_focus(id)
							end
							local new_item_id= disp.scroller.menu_pos
							if old_disp_id ~= id or old_item_id ~= new_item_id then
								self:update_cursor()
								if old_item_id > new_item_id then
									return "cursor_down"
								else
									return "cursor_up"
								end
							end
						end
					end
				end
			end
		end,
		mouse_inside= function(self, mx, my)
			if #self.menu_stack < 1 then return end
			for id= 0, self.scroller.num_main-1 do
				local disp= self.scroller.main_items[id]
				if disp.info and disp:check_focus(mx, my) then
					return true
				end
			end
		end,
		update_cursor= function(self)
			if not self.cursor then return end
			if not self.info then return end
			local item= self:get_cursor_item()
			if not item then
				local cursor_display= self.scroller:get_cursor_item()
				lua.ReportScriptError("disp: " .. self.scroller.menu_pos .. " item: " .. cursor_display.scroller.menu_pos)
				return
			end
			local ix, iy= rec_calc_actor_pos(item.container)
			local tx, ty= rec_calc_actor_pos(self.container)
			local cx= ix - tx
			local cy= iy - ty
			self.cursor:playcommand("Move", {x= cx, y= cy, actor= item.container})
		end,
		gain_focus= function(self)
			if not self.info then return end
			self.scroller:get_cursor_item():gain_focus()
		end,
		lose_focus= function(self)
			if not self.info then return end
			self.scroller:get_cursor_item():lose_focus()
		end,
		hide= function(self)
			self.hidden= true
			self.container:play_command_no_recurse("Hide")
		end,
		show= function(self)
			self.hidden= false
			self.container:play_command_no_recurse("Show")
		end,
}}

-- It's only 2 dimensions of the cross product because this is solely for
-- drawing outlines around the clickable area, which doesn't need 3D.
local function cross_product(va, vb)
	return {
		(va[2] * vb[3]) - (va[3] * vb[2]),
		(va[3] * vb[1]) - (va[1] * vb[3]),
	}
end

local function normalize(v, mag)
	local len= math.sqrt((v[1] * v[1]) + (v[2] * v[2])) * mag
	return {v[1] / len, v[2] / len}
end

function add_line_to_verts(pa, pb, verts, vc, thick, mag)
	local to= {pb[1] - pa[1], pb[2] - pa[2], 0}
	local left= normalize(cross_product(to, {0, 0, 1}), mag)
	local ht= thick * .5
	verts[#verts+1]= {{pa[1] + left[1], pa[2] + left[2], 0}, vc}
	verts[#verts+1]= {{pb[1] + left[1], pb[2] + left[2], 0}, vc}
	verts[#verts+1]= {{pb[1] - left[1], pb[2] - left[2], 0}, vc}
	verts[#verts+1]= {{pa[1] - left[1], pa[2] - left[2], 0}, vc}
end

function add_area_to_verts(area, verts, vc, thick)
	if #area < 2 then return end
	thick= thick or .5
	local mag= 1 / thick
	for p= 2, #area do
		add_line_to_verts(area[p-1], area[p], verts, vc, thick, mag)
	end
	add_line_to_verts(area[#area], area[1], verts, vc, thick, mag)
end

local function button_debug_actor()
	return Def.ActorMultiVertex{
		Name= "buttons_debug",
		InitCommand= function(self)
			self:SetDrawState{Mode= "DrawMode_Quads"}
		end,
		FrameCommand= function(self, menus)
			local verts= {}
			for pn, menu in pairs(menus) do
				local vc= menu.debug_button_color or {.75, 0, 0, 1}
				local displays= menu.scroller.main_items
				for did= 0, menu.scroller.num_main-1 do
					local disp= displays[did]
					local items= disp.scroller.main_items
					for iid= 0, disp.scroller.num_main-1 do
						local item= items[iid]
						if item.info ~= nil then
							for cid= 1, #item.buttons do
								add_area_to_verts(item.buttons[cid].actor:get_screen_clickable_area(), verts, vc)
							end
						end
					end
				end
			end
			self:SetVertices(verts)
		end,
	}
end

local function focus_debug_actor()
	return Def.ActorMultiVertex{
		Name= "focus_debug",
		InitCommand= function(self)
			self:SetDrawState{Mode= "DrawMode_Quads"}
		end,
		FrameCommand= function(self, menus)
			local verts= {}
			for pn, menu in pairs(menus) do
				local vc= menu.debug_focus_color or {0, .75, 0, 1}
				local sca= menu.debug_scroll_color or {0, 0, .75, 1}
				local displays= menu.scroller.main_items
				for did= 0, menu.scroller.num_main-1 do
					local disp= displays[did]
					add_area_to_verts(disp.container:get_screen_clickable_area(), verts, vc)
					if disp.mouse_scroll_area then
						add_area_to_verts(disp.mouse_scroll_area:get_screen_clickable_area(), verts, sca)
					end
					local items= disp.scroller.main_items
					for iid= 0, disp.scroller.num_main-1 do
						local item= items[iid]
						if item.info ~= nil then
							add_area_to_verts(item.container:get_screen_clickable_area(), verts, vc)
						end
					end
				end
			end
			self:SetVertices(verts)
		end,
	}
end

-- Part 1.5: Click area functions

click_area= {
	wh= function(self, w, h)
		self:set_clickable_area{{-w, -h}, {w, -h}, {w, h}, {-w, h}}
	end,
	owh= function(self, w, h)
		self:set_clickable_area{{0, -h}, {w, -h}, {w, h}, {0, h}}
	end,
	woh= function(self, w, h)
		self:set_clickable_area{{-w, 0}, {w, 0}, {w, h}, {-w, h}}
	end,
	owoh= function(self, w, h)
		self:set_clickable_area{{0, 0}, {w, 0}, {w, h}, {0, h}}
	end,
	lwh= function(self, l, w, h)
		self:set_clickable_area{{l, -h}, {l+w, -h}, {l+w, h}, {l, h}}
	end,
	ltwh= function(self, l, t, w, h)
		self:set_clickable_area{{l, t}, {l+w, t}, {l+w, t+h}, {l, t+h}}
	end,
}

-- Part 2: Functions that build certain types of menus and menu items.
-- Mostly to cover common cases of using lua config.

function npl_clamp(v, l, h)
	if h then
		v= math.min(v, h)
	end
	if l then
		v= math.max(v, l)
	end
	return v
end

local function find_choice(choice, reset, set)
	local rid= 1
	for i= 1, #set do
		if set[i] == choice then return i end
		if set[i] == reset then rid= i end
	end
	return rid
end

local function find_paired_choice(choice, reset, set)
	local rid= 1
	for i= 1, #set do
		if set[i][2] == choice then return i end
		if set[i][2] == reset then rid= i end
	end
	return rid
end

local function advance_choice(choice, dir, big, big_step, limit)
	if big_step and big then
		choice= choice + (big_step * dir)
	else
		choice= choice + dir
	end
	return one_index_is_a_mistake(choice, limit)
end

local function vtype_ret(val, vtype)
	if vtype then return {vtype, val} end
	return {type(val), val}
end

local function make_generic_reset(params)
	if params.reset == nil then return nil end
	if type(params.reset) == "function" then return params.reset end
	return function(arg, pn)
		params.set(arg, params.reset, pn)
		return vtype_ret(params.reset, params.value_type)
	end
end

local function make_custom_reset(params, custom)
	if params.reset == nil then return nil end
	if type(params.reset) == "function" then return params.reset end
	return custom
end

local function combine_menu_params(specific, generic)
	local combined= {}
	for name, thing in pairs(specific) do
		combined[name]= thing
	end
	for name, thing in pairs(generic) do
		if combined[name] == nil then
			combined[name]= thing
		end
	end
	return combined
end

local function add_broad_params(params, broad)
	if type(params) ~= "table" then params= {} end
	if not broad then return params end
	for name, par in pairs(broad) do
		if name ~= "main_type" and name ~= "broad_type" and
		params[name] == nil then
			params[name]= par
		end
	end
	return params
end

local function copy_parts(to, from, parts)
	for i, name in ipairs(parts) do
		to[name]= from[name]
	end
	return to
end

local function add_translation_params(entry, params)
	return copy_parts(entry, params, {"translation_section", "dont_translate_name", "dont_translate_value", "explanation"})
end

local function one_index_is_a_mistake(i, n)
	return ((i-1)%n)+1
end

local function pops_get(pn)
	return GAMESTATE:GetPlayerState(pn):get_player_options_no_defect(
		"ModsLevel_Preferred")
end

local function sops_get()
	return GAMESTATE:GetSongOptionsObject("ModsLevel_Preferred")
end

local broad_types= {
	number= {main_type= "number", small_step= 1, big_step= 10, reset= 0},
	bool= {main_type= "bool"},
	choice= {main_type= "choice"},
	toggle_number= {main_type= "toggle_number", on= 1, off= 0},
	name_value_pairs= {main_type= "name_value_pairs"},
	mistake= {main_type= "mistake"},

	millisecond= {main_type= "number", small_step= .001, big_step= .01, reset= 0, value_type= "ms"},
	percent= {main_type= "number", small_step= .01, big_step= .1, reset= 0, value_type= "percent"},
	time= {main_type= "number", small_step= 10, big_step= 30, min= 0, value_type= "time"},
	small_number= {main_type= "number", small_step= .1, big_step= 1, reset= 0},
	large_number= {main_type= "number", small_step= 10, big_step= 100, reset= 0},
}

local fallback_option_data= {
	preference= dofile("Themes/_fallback/Other/preference_menu_data.lua"),
	profile= dofile("Themes/_fallback/Other/profile_menu_data.lua"),
	player_option= dofile("Themes/_fallback/Other/player_menu_data.lua"),
	song_option= dofile("Themes/_fallback/Other/song_menu_data.lua"),
}

local submenu_puts_closure_at_top= true
local default_close_text= "&leftarrow;"

local menu_generics= {
	number= function(params)
		-- {name, get, set, arg, small_step, big_step, min, max, reset, sub_type, value_type}
		return {
			name= params.name, arg= params.arg, refresh= params.refresh,
			adjust= function(direction, big, arg, pn)
				local amount= direction * params.small_step
				if big and params.big_step then
					amount= direction * params.big_step
				end
				local new_value= npl_clamp(params.get(arg, pn) + amount, params.min, params.max)
				params.set(arg, new_value, pn)
				return vtype_ret(new_value, params.value_type)
			end,
			value= function(arg, pn)
				return vtype_ret(params.get(arg, pn), params.value_type)
			end,
			reset= make_generic_reset(params),
			type_hint= {main= "number", sub= params.sub_type},
		}
	end,
	bool= function(params)
		-- {name, get, set, arg, reset, sub_type}
		return {
			name= params.name, arg= params.arg, refresh= params.refresh,
			func_changes_value= true,
			func= function(big, arg, pn)
				local new_value= not params.get(arg, pn)
				params.set(arg, new_value, pn)
				return vtype_ret(new_value, params.value_type)
			end,
			value= function(arg, pn)
				return vtype_ret(params.get(arg, pn), params.value_type)
			end,
			reset= make_generic_reset(params),
			type_hint= {main= "bool", sub= params.sub_type},
		}
	end,
	choice= function(params)
		-- {name, get, set, arg, choices, big_step, reset, sub_type}
		return {
			name= params.name, arg= params.arg, refresh= params.refresh,
			func= function(big, arg, pn)
				local function gen_items()
					local items= {}
					for i, choice in ipairs(params.choices) do
						items[#items+1]= {
							name= choice, translation_section= params.translation_section,
							dont_translate_name= params.dont_translate_value,
							arg= params.arg, refresh= params.refresh,
							value= function(arg, pn)
								local curr_choice= params.get(arg, pn)
								if curr_choice == choice then
									return {"boolean", true}
								end
							end,
							func= function(big, arg, pn)
								params.set(arg, choice, pn)
							end,
						}
					end
					return nesty_menus.add_close_item(items)
				end
				return "submenu", gen_items()
			end,
			adjust= function(direction, big, arg, pn)
				local choice_id= find_choice(params.get(arg, pn), params.reset, params.choices)
				choice_id= advance_choice(choice_id, direction, big, params.big_step, #params.choices)
				local new_choice= params.choices[choice_id]
				params.set(arg, new_choice, pn)
				local trans_choice= new_choice
				return vtype_ret(new_choice, params.value_type)
			end,
			value= function(arg, pn)
				return vtype_ret(params.get(arg, pn), params.value_type)
			end,
			reset= make_generic_reset(params),
			type_hint= {main= "choice", sub= params.sub_type},
		}
	end,
	toggle_number= function(params)
		-- {name, get, set, arg, on, off, reset, sub_type, value_type}
		return {
			name= params.name, arg= params.arg, refresh= params.refresh,
			func_changes_value= true,
			func= function(big, arg, pn)
				local old_value= params.get(arg, pn)
				local mid= (params.on + params.off) * .5
				local new_value= old_value
				local disp= false
				if params.on > params.off then
					if new_value < mid then
						new_value= params.on
						disp= true
					else
						new_value= params.off
					end
				else
					if new_value < mid then
						new_value= params.off
					else
						new_value= params.on
						disp= true
					end
				end
				params.set(arg, new_value, pn)
				return vtype_ret(disp)
			end,
			value= function(arg, pn)
				local value= params.get(arg, pn)
				local mid= (params.on + params.off) * .5
				local disp= false
				if params.on > params.off then
					if value > mid then
						disp= true
					end
				else
					if value < mid then
						disp= true
					end
				end
				return vtype_ret(disp)
			end,
			reset= make_generic_reset(params),
			type_hint= {main= "toggle_number", sub= params.sub_type},
		}
	end,
	name_value_pairs= function(params)
		-- {name, get, set, arg, choices, big_step, reset, sub_type, value_type}
		return {
			name= params.name, arg= params.arg, refresh= params.refresh,
			func= function(big, arg, pn)
				local function gen_items()
					local items= {}
					for i, choice in ipairs(params.choices) do
						items[#items+1]= {
							name= choice[1],
							translation_section= params.translation_section,
							dont_translate_name= params.dont_translate_value,
							arg= params.arg, refresh= params.refresh,
							value= function(arg, pn)
								local curr_choice= params.get(arg, pn)
								if curr_choice == choice[2] then
									return {"boolean", true}
								end
							end,
							func= function(big, arg, pn)
								params.set(arg, choice[2], pn)
							end,
						}
					end
					return nesty_menus.add_close_item(items)
				end
				return "submenu", gen_items()
			end,
			adjust= function(direction, big, arg, pn)
				local choice_id= find_paired_choice(params.get(arg, pn), params.reset, params.choices)
				choice_id= advance_choice(choice_id, direction, big, params.big_step, #params.choices)
				local new_choice= params.choices[choice_id]
				params.set(arg, new_choice[2], pn)
				return vtype_ret(new_choice[1], params.value_type)
			end,
			value= function(arg, pn)
				local choice_id= find_paired_choice(params.get(arg, pn), params.reset, params.choices)
				local new_choice= params.choices[choice_id]
				return vtype_ret(new_choice[1], params.value_type)
			end,
			reset= make_custom_reset(
				params, function(arg, pn)
					local choice_id= find_paired_choice(params.reset, params.reset, params.choices)
					local new_choice= params.choices[choice_id]
					
					params.set(arg, new_choice[2], pn)
					return vtype_ret(new_choice[1], params.value_type)
			end),
			type_hint= {main= "name_value", sub= params.sub_type},
		}
	end,
	mistake= function()
		return {name= "themer mistake", func= function() SCREENMAN:SystemMessage("LOL") end, type_hint= {main= "mistake"}}
	end,
}

local function make_refresh(param_refresh, parts)
	local ref= parts
	if type(param_refresh) == "table" then
		for name, part in pairs(params.refresh) do
			ref[name]= part
		end
	end
	return ref
end

local function menu_message(params)
	MESSAGEMAN:Broadcast("MenuValueChanged", params)
end

local menu_specifics= {
	preference= {
		arg= function(params)
			return params.name
		end,
		refresh= function(params)
			return make_refresh(
				params.refresh, {category= "Preference", field_name= params.name})
		end,
		get= function(name)
			return PREFSMAN:GetPreference(name)
		end,
		set= function(name, value)
			PREFSMAN:SetPreference(name, value)
			menu_message{category= "Preference", field_name= name, value= value}
		end,
		reset= function(name, value)
			if value == nil then return PREFSMAN:GetPreference(name) end
			return value
		end,
	},
	config= {
		arg= function(params)
			return {config= params.config, path= params.path, value_type= params.value_type}
		end,
		refresh= function(params)
			return make_refresh(
				params.refresh, {
					category= "Config", config_name= params.config.name,
					field_name= params.path,
			})
		end,
		get= function(arg, pn)
			return get_element_by_path(arg.config:get_data(pn), arg.path)
		end,
		set= function(arg, value, pn)
			set_element_by_path(arg.config:get_data(pn), arg.path, value)
			arg.config:set_dirty(pn)
			menu_message{
				category= "Config", config_name= arg.config.name,
				field_name= arg.path, value= value, pn= pn}
		end,
		reset= function(name, value)
			if value ~= nil then return value end
			return function(arg, pn)
				local new_value= get_element_by_path(arg.config:get_default(), arg.path)
				set_element_by_path(arg.config:get_data(pn), arg.path, new_value)
				arg.config:set_dirty(pn)
				menu_message{
					category= "Config", config_name= arg.config.name,
					field_name= arg.path, value= new_value, pn= pn}
				return vtype_ret(new_value, arg.value_type)
			end
		end,
	},
	profile= {
		arg= function(params)
			return params.name
		end,
		refresh= function(params)
			return make_refresh(
				params.refresh, {
					category= "Profile", field_name= params.name, match_pn= true,
			})
		end,
		get= function(name, pn)
			local profile= PROFILEMAN:GetProfile(pn)
			return profile["Get"..name](profile)
		end,
		set= function(name, value, pn)
			local profile= PROFILEMAN:GetProfile(pn)
			profile["Set"..name](profile, value)
			menu_message{category= "Profile", field_name= name, pn= pn, value= value}
		end,
	},
	data= {
		arg= function(params)
			return {data= params.data, path= params.path}
		end,
		refresh= function(params)
			return make_refresh(
				params.refresh, {
					category= "Data", data_name= params.data.name,
					field_name= params.path,
			})
		end,
		get= function(arg, pn)
			return get_element_by_path(arg.data, arg.path)
		end,
		set= function(arg, value, pn)
			set_element_by_path(arg.data, arg.path, value)
			menu_message{
				category= "Data", data_name= arg.data.name, field_name= arg.path,
				value= value, pn= pn}
		end,
	},
	player_option= {
		arg= function(params)
			return params.name
		end,
		refresh= function(params)
			return make_refresh(
				params.refresh, {
					category= "PlayerOption", field_name= params.name,
			})
		end,
		get= function(name, pn)
			local ops= pops_get(pn)
			return ops[name](ops)
		end,
		set= function(name, value, pn)
			local ops= pops_get(pn)
			-- We need to inform GameState if we set the fail type so it doesn't
			-- override it with the beginner/easy preferences.
			if name == "FailSetting" then
				GAMESTATE:SetFailTypeExplicitlySet()
			end
			ops[name](ops, value)
			menu_message{
				category= "PlayerOption", pn= pn, field_name= name, value= value}
		end,
	},
	song_option= {
		arg= function(params)
			return params.name
		end,
		refresh= function(params)
			return make_refresh(
				params.refresh, {category= "SongOption", field_name= params.name})
		end,
		get= function(name)
			local sops= sops_get()
			return sops[name](sops)
		end,
		set= function(name, value)
			local sops= sops_get()
			sops[name](sops, value)
			-- Apply the change to the current and song levels too, so that if
			-- this occurs during gameplay, it takes effect immediately.
			GAMESTATE:ApplyPreferredSongOptionsToOtherLevels()
			menu_message{category= "SongOption", field_name= name, value= value}
		end,
		reset= function(name, value)
			if value == nil then
				local sops= sops_get()
				return sops[name](sops)
			end
			return value
		end,
	},
}

nesty_menus= {
	menu_message= menu_message,
	button_debug_actor= button_debug_actor,
	focus_debug_actor= focus_debug_actor,
	add_broad_type= function(name, params)
		broad_types[name]= params
	end,
	set_close_default_to_top= function(val)
		submenu_puts_closure_at_top= val
	end,
	set_default_close_text= function(text)
		default_close_text= text
	end,
	close_item= function(name)
		return {
			name= name, dont_translate_name= true, type_hint= {main= "close"},
			func= function() return "close", 1 end}
	end,
	add_close_item= function(items, close_name, as_custom)
		local name= close_name or default_close_text
		if not name then return items end
		local found_closure= false
		for i= 1, #items do
			local it= items[i]
			if it.type_hint == "close" then
				found_closure= true
				break
			elseif type(it.type_hint) == "table" then
				if it.type_hint.main == "close" then
					found_closure= true
					break
				end
			end
		end
		if not found_closure then
			local closure= nesty_menus.close_item(name)
			if as_custom then
				closure= {"custom", closure}
			end
			if submenu_puts_closure_at_top then
				table.insert(items, 1, closure)
			else
				items[#items+1]= closure
			end
		end
		return items
	end,
	item= function(sub, name, broad, params)
		local sub_type= sub
		if type(sub) == "table" then
			if sub.is_lua_config then
				sub_type= "config"
				params= add_broad_params(params, {config= sub, path= name})
			else
				sub_type= "data"
				params= add_broad_params(params, {data= sub, path= name})
			end
		end
		local broad_params= broad_types[broad]
		if not broad_params then
			local option_data_lookup= fallback_option_data[sub_type]
			if option_data_lookup then
				local data_entry= option_data_lookup[name]
				if data_entry then
					broad_params= broad_types[data_entry.broad_type]
					params= add_broad_params(params, data_entry)
				end
			end
		end
		if not broad_params then
			Trace("Invalid item: " .. tostring(sub) .. ", " .. tostring(name) .. ", " .. tostring(broad))
		end
		assert(broad_params, "No clue what you're trying to pull.")
		params= add_broad_params(params, broad_params)
		-- name will be translated in item_controller:set_info because the theme
		-- needs to be able to search for items by name if that feature is added.
		params.name= name
		local main_type= params.main_type or broad_params.main_type
		local generic_entry= menu_generics[main_type]
		if sub_type == "generic" then
			return add_translation_params(generic_entry(params), params)
		end
		local specific_entry= menu_specifics[sub_type]
		if not generic_entry or not specific_entry then
			return menu_generics.mistake()
		end
		local reset= nil
		if specific_entry.reset then
			reset= specific_entry.reset(params.name, params.reset)
		end
		local refresh= params.refresh
		if specific_entry.refresh and type(refresh) ~= "function" then
			refresh= specific_entry.refresh(params)
		end
		local ret= generic_entry(
			combine_menu_params(
				params, {
					get= specific_entry.get, set= specific_entry.set, reset= reset,
					arg= specific_entry.arg(params), refresh= refresh,
					sub_type= sub_type}))
		add_translation_params(ret, params)
		return setmetatable(ret, mergable_table_mt)
	end,
	submenu= function(name, items, params)
		if not params.no_close then
			nesty_menus.add_close_item(items)
		end
		local ret= {
			name= name, func= function() return "submenu", items end,
			type_hint= {main= "submenu", sub= params.sub_type},
		}
		add_translation_params(ret, params)
		return setmetatable(ret, mergable_table_mt)
	end,
	make_menu= function(info)
		if not info then return end
		local ret= {}
		copy_parts(ret, info, {"on_open", "on_open_arg", "on_close", "on_close_arg"})
		for eid= 1, #info do
			local entry= info[eid]
			if type(entry) ~= "table" then
				Trace("All menu entries must be tables.")
				rec_print_table(info, nil, 1)
				assert(false, "Menu entry " .. eid .. " cannot hold drinks or food.")
			end
			local entype= entry[1]
			local type_handlers= {
				close= function()
					return nesty_menus.close_item(entry[2] or default_close_text)
				end,
				item= function()
					return nesty_menus.item(entry[2], entry[3], entry[4], entry[5])
				end,
				submenu= function()
					local name= entry[2]
					local items= nesty_menus.make_menu(entry[3])
					local sub_type= entry[4]
					if not entry.no_close then
						nesty_menus.add_close_item(items)
					end
					local ret= {
						name= name, func= function() return "submenu", items end,
						type_hint= {main= "submenu", sub= sub_type},
					}
					return add_translation_params(ret, entry)
				end,
				action= function()
					return add_translation_params({
						name= entry[2], func= entry[3], arg= entry[4],
						type_hint= {main= "action", sub= entry[5]}}, entry)
				end,
				custom= function()
					return entry[2]
				end,
			}
			local handler= type_handlers[entype]
			if not handler then
				Trace("Invalid menu entry:")
				rec_print_table(entry)
			end
			assert(handler, "Menu entry " .. eid .. " is of unknown type: " .. tostring(entype))
			local success, item= pcall(handler)
			assert(success, "Menu entry " .. eid .. " had problem..."..tostring(item))
			ret[#ret+1]= item
		end
		return ret
	end,
}

-- Part 3: An actor to wrap around attaching the menu and the typical input
-- and update functions.

nesty_menus.play_menu_sound= function(sounds, name)
	if not name or not sounds then return end
	local sound_entry= sounds[nesty_menus.action_to_sound_name[name]]
	if sound_entry then
		sound_entry:play()
	end
end

nesty_menus.find_menu_for_mouse= function(menus, mx, my)
	for pn, menu in pairs(menus) do
		if menu:mouse_inside(mx, my) then
			return pn, menu
		end
	end
end

local function make_typical_update(menu_controllers, sounds, item_change_callback, buttons_debug, focus_debug)
	local prev_mx= INPUTFILTER:GetMouseX()
	local prev_my= INPUTFILTER:GetMouseY()
	local function common()
		local mx= INPUTFILTER:GetMouseX()
		local my= INPUTFILTER:GetMouseY()
		if mx ~= prev_mx or my ~= prev_my then
			local pn, menu= nesty_menus.find_menu_for_mouse(menu_controllers, mx, my)
			if menu then
				local sound_name= menu:update_focus(mx, my)
				nesty_menus.play_menu_sound(sounds, sound_name)
				if item_change_callback then
					item_change_callback(menu:get_cursor_item(), pn)
				end
			end
			prev_mx= mx
			prev_my= my
		end
	end
	if buttons_debug or focus_debug then
		return function()
			common()
			if buttons_debug then
				buttons_debug:playcommand("Frame", menu_controllers)
			end
			if focus_debug then
				focus_debug:playcommand("Frame", menu_controllers)
			end
		end
	else
		return common
	end
end

local function make_typical_input(menu_controllers, sounds, item_change_callback, exit_callback)
	return function(event)
		local pn, menu
		if event.DeviceInput.is_mouse then
			local mx= INPUTFILTER:GetMouseX()
			local my= INPUTFILTER:GetMouseY()
			pn, menu= nesty_menus.find_menu_for_mouse(menu_controllers, mx, my)
		else
			pn= event.PlayerNumber
			menu= menu_controllers[pn] or menu_controllers[1]
		end
		if menu then
			local levels_left, sound_name= menu:input(event)
			nesty_menus.play_menu_sound(sounds, sound_name)
			if item_change_callback then
				item_change_callback(menu:get_cursor_item(), pn)
			end
			if levels_left and levels_left < 1 then
				if exit_callback then
					exit_callback(pn)
				end
			end
		end
	end
end

nesty_menus.action_to_sound_name= {
	action= "toggle",
	cursor_down= "down",
	cursor_up= "up",
	page_down= "down",
	page_up= "up",
	jump_top= "up",
	jump_bottom= "down",
	adjust_down= "decrease",
	adjust_up= "increase",
	adjust_mode_on= "toggle",
	adjust_mode_off= "toggle",
	open_submenu= "confirm",
	close_submenu= "back",
	close_menu= "back",
}

local sound_names= {
	"up", "down", "increase", "decrease", "toggle", "confirm", "back",
}

nesty_menus.load_typical_menu_sounds= function()
	local frame= Def.ActorFrame{Name= "sounds"}
	for i, name in ipairs(sound_names) do
		local path= THEME:GetPathS("OptionMenu", name, true)
		if path ~= "" then
			frame[#frame+1]= LoadActor(path) .. {Name= "sound_"..name}
		end
	end
	if #frame < 1 then return end
	return frame
end

nesty_menus.make_menu_sound_lookup= function(self)
	local container= self:GetChild("sounds")
	if not container then return end
	local sound_actors= {}
	for i, name in ipairs(sound_names) do
		local actor= container:GetChild("sound_"..name)
		if actor then
			sound_actors[name]= actor
		end
	end
	return sound_actors
end

nesty_menus.handle_menu_refresh_message= function(message_param, menu_controllers)
	local mpnt_isn_str= type(message_param.pn) ~= "string"
	for pn, controller in pairs(menu_controllers) do
		local pnt_isn_str= type(pn) ~= "string"
		for did= 0, controller.scroller.num_main-1 do
			local disp= controller.scroller.main_items[did]
			if disp.info and #disp.info > 1 then
				for iid= 0, disp.scroller.num_main-1 do
					local item= disp.scroller.main_items[iid]
					if item.info then
						local refresh= item.info.refresh
						if refresh then
							local should_refresh= false
							if type(refresh) == "function" then
								should_refresh= refresh(item.info, message_param)
							else
								if not refresh.match_pn or pnt_isn_str or mpnt_isn_str or pn == message_param.pn then
									local parts_match= true
									for name, value in pairs(message_param) do
										if refresh[name] ~= nil and refresh[name] ~= value then
											parts_match= false
										end
									end
									should_refresh= parts_match
								end
							end
							if should_refresh then
								item:refresh_value()
							end
						end
					end
				end
			end
		end
	end
end

nesty_menus.make_menu_actors= function(menu_args)
	-- actors, data, input_mode, repeats_to_big, select_goes_to_top, dont_open,
	-- item_change_callback, exit_callback, translation_section,
	-- with_click_debug
	local menu_controllers= {}
	local frame= Def.ActorFrame{
		OnCommand= function(self)
			local sound_actors= nesty_menus.make_menu_sound_lookup(self)
			for pn, temp in pairs(menu_controllers) do
				local menu= self:GetChild("menu_"..pn)
				local controller= menu_controllers[pn]
				local sub_args= copy_parts({}, menu_args, {"input_mode", "repeats_to_big", "select_goes_to_top", "data", "translation_section"})
				sub_args.actor= menu
				sub_args.pn= pn
				controller:init(sub_args)
				if not menu_args.dont_open then
					controller:open_menu()
					if menu_args.item_change_callback then
						menu_args.item_change_callback(controller:get_cursor_item(), pn)
					end
				end
				if menu_args.with_click_debug then
					controller.debug_button_color= {.75, 0, 0, 1}
					controller.debug_focus_color= {0, .75, 0, 1}
					controller.debug_scroll_color= {0, 0, .75, 1}
				end
			end
			local input= make_typical_input(
				menu_controllers, sound_actors, menu_args.item_change_callback,
				menu_args.exit_callback)
			SCREENMAN:GetTopScreen():AddInputCallback(input)
			local buttons_debug= self:GetChild("buttons_debug")
			local focus_debug= self:GetChild("focus_debug")
			local update= make_typical_update(
				menu_controllers, sound_actors, menu_args.item_change_callback,
				buttons_debug, focus_debug)
			self:SetUpdateFunction(update)
		end,
		MenuValueChangedMessageCommand= function(self, params)
			nesty_menus.handle_menu_refresh_message(params, menu_controllers)
		end,
	}
	local sounds= nesty_menus.load_typical_menu_sounds()
	if sounds then
		frame[#frame+1]= sounds
	end
	for pn, temp in pairs(menu_args.actors) do
		menu_controllers[pn]= setmetatable({}, menu_controller_mt)
		temp.Name= "menu_"..pn
		frame[#frame+1]= temp
	end
	if menu_args.with_click_debug then
		frame[#frame+1]= nesty_menus.button_debug_actor()
		frame[#frame+1]= nesty_menus.focus_debug_actor()
	end
	return frame, menu_controllers
end

-- When the user goes up or down a page, or wraps from top to bottom, "from"
-- or "to" will be far outside the range of 1 to num_items.  It looks bad
-- when the item appears at the from position when that's outside the menu
-- area, or moves all the way to the to position before disappearing.
-- So this function takes care of hiding the item when it's outside the menu
-- area.
-- Maybe masking would be better. >_>
function one_dimension_scroll(
		self, dim, tween, time, spacing, from, to, low, high)
	if from == to then
		self[dim](self, from * spacing)
		self:diffusealpha(1)
		return
	end
	self:finishtweening()
	local steps= math.abs(to-from)
	local time_per_step= time / steps
	local dir= from < to and 1 or -1
	local visible_from= clamp(from, low, high)
	local steps_before_visible= math.abs(visible_from - from)
	local steps_before_fade_in= steps_before_visible - 1
	if steps_before_visible > 0 then
		local pos_before_vis= visible_from - dir
		self[dim](self, pos_before_vis * spacing)
		if steps_before_fade_in > 0 then
			self:sleep(steps_before_fade_in * time_per_step)
		end
		self[tween](self, time_per_step)
		self:diffusealpha(1)
		self[dim](self, visible_from * spacing)
	end
	local visible_to= clamp(to, low, high)
	local steps_after_visible= math.abs(visible_to - to)
	local steps_visible= math.abs(visible_to - visible_from)
	if steps_visible > 0 then
		self[tween](self, steps_visible * time_per_step)
		self[dim](self, visible_to * spacing)
	end
	if steps_after_visible > 0 then
		local pos_after_vis= visible_to + dir
		self[tween](self, time_per_step)
			:diffusealpha(0)
		self[dim](self, pos_after_vis * spacing)
	end
end

function add_defaults_to_params(params, defaults)
	if type(params) ~= "table" then params= {} end
	if type(defaults) ~= "table" then return params end
	for key, value in pairs(defaults) do
		if params[key] == nil then params[key]= value end
	end
	return params
end

function add_blank_tables_to_params(params, table_names)
	if type(params) ~= "table" then params= {} end
	if type(table_names) ~= "table" then return params end
	for i, name in ipairs(table_names) do
		if not params[name] then params[name]= {} end
	end
	return params
end

function noop_nil() end

function rec_calc_actor_extent(aframe, depth)
	depth= depth or ""
	if not aframe then return 0, 0, 0, 0 end
	local halign= aframe:GetHAlign()
	local valign= aframe:GetVAlign()
	local w= aframe:GetZoomedWidth()
	local h= aframe:GetZoomedHeight()
	local halignjust= (halign - .5) * w
	local valignjust= (valign - .5) * h
	local xmin= w * -halign
	local xmax= w * (1 - halign)
	local ymin= h * -valign
	local ymax= h * (1 - valign)
	local function handle_child(xz, yz, child)
		if child:GetVisible() then
			local cx= child:GetX() + halignjust
			local cy= child:GetY() + valignjust
			--Trace(depth .. "child " .. child:GetName() .. " at " .. cx .. ", " .. cy)
			local cxmin, cxmax, cymin, cymax= rec_calc_actor_extent(child,depth.."  ")
			xmin= math.min((cxmin * xz) + cx, xmin)
			ymin= math.min((cymin * yz) + cy, ymin)
			xmax= math.max((cxmax * xz) + cx, xmax)
			ymax= math.max((cymax * yz) + cy, ymax)
		end
	end
	if aframe.GetChildren then
		local xz= aframe:GetZoomX()
		local yz= aframe:GetZoomY()
		local children= aframe:GetChildren()
		for i, c in pairs(children) do
			if #c > 0 then
				for subi= 1, #c do
					handle_child(xz, yz, c[subi])
				end
			else
				handle_child(xz, yz, c)
			end
		end
	else
		--Trace(depth .. "no children")
	end
	--Trace(depth .. "rec_calc_actor_extent:")
	--Trace(depth .. "ha: " .. halign .. " va: " .. valign .. " w: " .. w ..
	--			" h: " .. h .. " haj: " .. halignjust .. " vaj: " .. valignjust ..
	--			" xmn: " .. xmin .. " xmx: " .. xmax .. " ymn: " .. ymin .. " ymx: "
	--			.. ymax)
	return xmin, xmax, ymin, ymax
end

function rec_calc_actor_pos(actor)
	-- This doesn't handle zooming.
	if not actor then return 0, 0 end
	local x= actor:GetDestX()
	local y= actor:GetDestY()
	local wx, wy= 0, 0
	if actor.GetNumWrapperStates then
		local wrappers= actor:GetNumWrapperStates()
		for i= 1, wrappers do
			local nitori= actor:GetWrapperState(i)
			wx= wx + nitori:GetDestX()
			wy= wy + nitori:GetDestY()
		end
	end
	local px, py= rec_calc_actor_pos(actor:GetParent())
	return x+px+wx, y+py+wy
end
