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
	gain_focus= function(self)
		self.container:play_command_no_recurse("GainFocus")
	end,
	lose_focus= function(self)
		self.container:play_command_no_recurse("LoseFocus")
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
			add_part(self, "adjust_up", clickables, self.adjust_up_click, nil, false, "adjust")
			add_part(self, "adjust_down", clickables, self.adjust_down_click, nil, false, "adjust")
			self.buttons= clickables
			self.container:playcommand("Playerize", self.pn)
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
					local section= info.translation_section or "OptionNames"
					trans_name= want_string(section, info.name)
				end
				self.name_actor:playcommand("SetName", trans_name)
				local value_type= type(info.value)
				if value_type == "nil" then
					self:set_value(nil)
					self.container:play_command_no_recurse("SetTypeHint", info.type_hint)
				elseif value_type == "function" then
					local display_value= info.value(info.arg, self.pn)
					self:set_value(display_value)
				else
					self:set_value(info.value)
				end
				show_hide_part(self, "adjust_up_actor", info.adjust)
				show_hide_part(self, "adjust_down_actor", info.adjust)
			else
				self.container:play_command_no_recurse("ClearItem")
			end
		end,
		check_click= function(self, press_info)
			if not self.info then return end
			for i= 1, #self.buttons do
				local entry= self.buttons[i]
				if self.info[entry.func_name] then
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
			return self.container:pos_in_clickable_area(mx, my)
		end,
		name_click= function(self, press_info)
			return self:interact(press_info.big_any)
		end,
		value_click= function(self, press_info)
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
			if type(v) == "table" then
				local value= v[2]
				v[3]= value
				if type(value) == "string"
				and not self.info.dont_translate_value then
					local section= self.info.translation_section or "OptionNames"
					v[2]= want_string(section, value)
				end
			end
			self.value_actor:playcommand("SetValue", v)
		end,
		interact= function(self, big)
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
			if self.info.reset then
				self.container:play_command_no_recurse("Reset")
				self:set_value(self.info.reset(self.info.arg, self.pn))
			end
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
		clamp_offset= function(self, offset)
			return clamp(offset, 0, #self.info - self.num_main)
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
						item:scroll(self.num_main, "first", id+1, id+1)
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
						item:scroll(self.num_main, "first", id+1, id+1)
						item:set_active()
					end
					for id= num_to_set, self.num_main-1 do
						local item= self.main_items[id]
						item:set_info(nil)
						item:scroll(self.num_main, "first", id+1, id+1)
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
					main:scroll(self.num_main, scroll_type, main_id+1, shift_to+1)
					spare:set_info(self.info[info_id])
					spare:scroll(self.num_main, "on", insert_from, main_id+1)
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
					self.menu_offset= math.max(0, #self.info - self.num_main)
					self:shift_all_items(1)
					if self.menu_offset + self.menu_pos >= #self.info then
						self.menu_pos= math.max(0, #self.info - self.menu_offset - 1)
					end
				else
					local old_offset= self.menu_offset
					if self.menu_offset + self.num_main >= #self.info then
						self.menu_offset= math.max(0, #self.info - self.num_main - 1)
					end
					-- TODO: Scroll items around nicely.
					for main_id= 0, self.num_main-1 do
						local main= self.main_items[main_id]
						local info_id= self.menu_offset + main_id + 1
						local info= self.info[info_id]
						if info ~= main.info then
							main:set_info(info)
						end
					end
				end
			end
			if self.menu_offset + self.menu_pos >= #self.info then
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
				local info_id= self.menu_offset + id + 1
				if self.wrapping then
					info_id= one_index_is_a_mistake(info_id, #self.info)
				end
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
				main:scroll(self.num_main, "off", id+1, id + dist + 1)
				local info_id= self:item_id_to_info_id(id)
				spare:set_info(self.info[info_id])
				spare:scroll(self.num_main, "on", id - dist + 1, id+1)
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
					item:scroll(self.num_main, "off", id+1, to+1)
					to_spare[#to_spare+1]= item
				else
					item:scroll(self.num_main, "normal", id+1, to+1)
					self.main_items[to]= item
				end
			end
			for id= 0, amount-1 do
				local spare= self.spare_items[id]
				local to= self.num_main - amount + id + 1
				local from= to + amount
				local info_id= self.menu_offset + to
				spare:set_info(self.info[info_id])
				spare:scroll(self.num_main, "on", from, to)
				self.main_items[to-1]= spare
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
					item:scroll(self.num_main, "off", id+1, to+1)
					to_spare[#to_spare+1]= item
				else
					item:scroll(self.num_main, "normal", id+1, to+1)
					self.main_items[to]= item
				end
			end
			for id= 0, amount-1 do
				local spare= self.spare_items[id]
				local to= id
				local from= to - amount
				local info_id= self.menu_offset + to + 1
				spare:set_info(self.info[info_id])
				spare:scroll(self.num_main, "on", from+1, to+1)
				self.main_items[id]= spare
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
						if self.menu_offset < #self.info - self.num_main then
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
			local max_offset= math.max(0, #self.info - self.num_main)
			local new_offset= clamp(self.menu_offset + dir, 0, max_offset)
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
				local cursor_info_pos= self:get_cursor_info_pos()
				local dist= pos - cursor_info_pos
				local new_offset= self.menu_offset
				local new_pos= self.menu_pos
				if self.wrapping then
					new_offset= (pos - self.focus_pos - 1) % #self.info
					new_pos= self.focus_pos
				else
					new_offset= self:clamp_offset(pos - self.focus_pos - 1)
					new_pos= pos - 1 - new_offset
				end
				self.menu_pos= new_pos
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
}}

for name, func in pairs(shared_functions) do
	item_controller_mt.__index[name]= func
	display_controller_mt.__index[name]= func
end

local menu_convert= {
	Start= "start",
	Select= "select",
	MenuLeft= "left",
	MenuRight= "right",
	MenuUp= "up",
	MenuDown= "down",
}
local page_convert= {
	DeviceButton_pgdn= "page_down",
	DeviceButton_pgup= "page_up",
}
menu_controller_mt= {
	__index= {
		input_modes= {
			two_direction= true, -- LR + Start
			two_direction_with_select= true, -- LR + Start + Select
			four_direction= true, -- LRUD + Start + Select
		},
		attach= function(self, container, pn)
			-- attach display_controllers to displays.
			-- store clickable info from menu items.
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
			self.repeats_to_big= repeats_to_big or 10
			self.select_goes_to_top= select_goes_to_top
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
							return
						end
					end
				end
				local action, info, more_info= disp:check_click(press_info)
				if action then
					if type(action) == "string" then
						local pop_to= self.scroller:item_id_to_info_id(id)
						self:pop_menus_to(id)
						return self:handle_menu_action(action, info, more_info)
					else
						return #self.menu_stack
					end
				end
			end
		end,
		menu_button= function(self, button, press_type)
			local process, big= self:handle_repeats(button, press_type)
			if not process then return end
			local active_display= self.scroller:get_cursor_item()
			if button == "page_down" then
				active_display:page_down()
				self:update_cursor()
			elseif button == "page_up" then
				active_display:page_up()
				self:update_cursor()
			else
				if self.input_mode == "two_direction" then
					if self.in_adjust_mode then
						if button == "left" then
							active_display:get_cursor_item():adjust(-1, big)
						elseif button == "right" then
							active_display:get_cursor_item():adjust(1, big)
						elseif button == "start" then
							self.in_adjust_mode= false
							if self.cursor then
								self.cursor:playcommand("NormalMode")
							end
						end
					else
						if button == "left" then
							active_display:cursor_up()
							self:update_cursor()
						elseif button == "right" then
							active_display:cursor_down()
							self:update_cursor()
						elseif button == "start" then
							local item= active_display:get_cursor_item()
							if item.info.func then
								return self:handle_menu_action(item:interact(big))
							elseif item.info.adjust then
								self.in_adjust_mode= true
								if self.cursor then
									self.cursor:playcommand("AdjustMode")
								end
							end
						end
					end
				elseif self.input_mode == "two_direction_with_select" then
					if button == "left" then
						active_display:get_cursor_item():adjust(-1, big)
					elseif button == "right" then
						local item= active_display:get_cursor_item()
						if item.info.func then
							return self:handle_menu_action(item:interact(big))
						else
							item:adjust(1, big) 
						end
					elseif button == "start" then
						active_display:cursor_down()
						self:update_cursor()
					elseif button == "select" then
						active_display:cursor_up()
						self:update_cursor()
					end
				elseif self.input_mode == "four_direction" then
					if button == "left" then
						active_display:get_cursor_item():adjust(-1, big)
					elseif button == "right" then
						active_display:get_cursor_item():adjust(1, big)
					elseif button == "up" then
						active_display:cursor_up()
						self:update_cursor()
					elseif button == "down" then
						active_display:cursor_down()
						self:update_cursor()
					elseif button == "start" then
						return self:handle_menu_action(
							active_display:get_cursor_item():interact(big))
					elseif button == "select" then
						if self.select_goes_to_top then
							active_display:cursor_to_top()
							self:update_cursor()
						else
							active_display:cursor_to_bottom()
							self:update_cursor()
						end
					end
				end
			end
		end,
		get_cursor_item= function(self)
			return self.scroller:get_cursor_item():get_cursor_item()
		end,
		handle_menu_action= function(self, action, info, extra)
			if not action then return end
			local active_display= self.scroller:get_cursor_item()
			if action == "refresh" then
				if type(info) == "table" then
					active_display:refresh_info(info)
					self:update_cursor()
				end
			elseif action == "submenu" then
				if type(info) == "table" then
					self:push_menu(info)
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
					active_display:refresh_info(extra)
					self:update_cursor()
				else
					if info < 0 or info >= #self.menu_stack then
						self:close_menu()
					else
						for i= 1, info do
							self:pop_menu()
						end
					end
				end
			end
			return #self.menu_stack
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
			self.scroller:cursor_down()
			self:update_cursor()
		end,
		pop_menu= function(self)
			if #self.menu_stack <= 1 then return end
			local being_popped= self.menu_stack[#self.menu_stack]
			local remembered_pos= being_popped.remembered_pos
			if being_popped.on_close then
				being_popped.on_close(being_popped.on_close_arg, remembered_pos, self.pn)
			end
			self.scroller:remove_info(#self.menu_stack)
			self:update_cursor()
		end,
		pop_menus_to= function(self, id)
			while #self.menu_stack > id+1 do
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
								return
							end
						end
					end
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
			self.container:play_command_no_recurse("Hide")
		end,
		show= function(self)
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

local function normalize(v)
	local len= math.sqrt((v[1] * v[1]) + (v[2] * v[2])) * 2
	return {v[1] / len, v[2] / len}
end

local function add_line_to_verts(pa, pb, verts, vc)
	local to= {pb[1] - pa[1], pb[2] - pa[2], 0}
	local left= normalize(cross_product(to, {0, 0, 1}))
	verts[#verts+1]= {{pa[1] + left[1], pa[2] + left[2], 0}, vc}
	verts[#verts+1]= {{pb[1] + left[1], pb[2] + left[2], 0}, vc}
	verts[#verts+1]= {{pb[1] - left[1], pb[2] - left[2], 0}, vc}
	verts[#verts+1]= {{pa[1] - left[1], pa[2] - left[2], 0}, vc}
end

local function add_area_to_verts(area, verts, vc)
	if #area < 2 then return end
	for p= 2, #area do
		add_line_to_verts(area[p-1], area[p], verts, vc)
	end
	add_line_to_verts(area[#area], area[1], verts, vc)
end

function menu_buttons_debug_actor()
	return Def.ActorMultiVertex{
		Name= "buttons_debug",
		InitCommand= function(self)
			self:SetDrawState{Mode= "DrawMode_Quads"}
		end,
		FrameCommand= function(self, menu)
			local verts= {}
			local vc= menu.debug_button_color or {.75, .75, .75, 1}
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
			self:SetVertices(verts)
		end,
	}
end

function menu_focus_debug_actor()
	return Def.ActorMultiVertex{
		Name= "focus_debug",
		InitCommand= function(self)
			self:SetDrawState{Mode= "DrawMode_Quads"}
		end,
		FrameCommand= function(self, menu)
			local verts= {}
			local vc= menu.debug_focus_color or {.75, .75, .75, 1}
			local sca= menu.debug_scroll_color or {.75, .75, .75, 1}
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

local function add_translation_params(entry, params)
	for i, name in ipairs{"translation_section", "dont_translate_name", "dont_translate_value"} do
		entry[name]= params[name]
	end
	return entry
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
local default_close_item= {
	name= "<-", dont_translate_name= true, type_hint= {main= "close"},
	func= function() return "close", 1 end,
}
local submenu_close_item= default_close_item

local menu_generics= {
	number= function(params)
		-- {name, get, set, arg, small_step, big_step, min, max, reset, sub_type, value_type}
		return {
			name= params.name, arg= params.arg,
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
			name= params.name, arg= params.arg,
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
			name= params.name, arg= params.arg,
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
			name= params.name, arg= params.arg,
			func_changes_value= true,
			func= function(big, arg, pn)
				local old_value= params.get(arg, pn)
				local mid= (params.on + params.off) * .5
				local new_value= old_value
				if num_val > mid then
					new_value= math.max(params.on, params.off)
				else
					new_value= math.min(params.on, params.off)
				end
				params.set(arg, new_value, pn)
				return vtype_ret(new_value, params.value_type)
			end,
			value= function(arg, pn)
				return vtype_ret(params.get(arg, pn), params.value_type)
			end,
			reset= make_generic_reset(params),
			type_hint= {main= "toggle_number", sub= params.sub_type},
		}
	end,
	name_value_pairs= function(params)
		-- {name, get, set, arg, choices, big_step, reset, sub_type, value_type}
		return {
			name= params.name, arg= params.arg,
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
		return {name= "themer mistake", func= function() SCREENMAN:SystemMessage("LOL") end}
	end,
}

local menu_specifics= {
	preference= {
		arg= function(params)
			return params.name
		end,
		get= function(name)
			return PREFSMAN:GetPreference(name)
		end,
		set= function(name, value)
			PREFSMAN:SetPreference(name, value)
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
		get= function(arg, pn)
			return get_element_by_path(arg.config:get_data(pn), arg.path)
		end,
		set= function(arg, value, pn)
			set_element_by_path(arg.config:get_data(pn), arg.path, value)
			arg.config:set_dirty(pn)
			MESSAGEMAN:Broadcast("ConfigValueChanged", {
				config_name= arg.config.name, field_name= arg.path,
				value= value, pn= pn})
		end,
		reset= function(name, value)
			if value ~= nil then return value end
			return function(arg, pn)
				local new_value= get_element_by_path(arg.config:get_default(), arg.path)
				set_element_by_path(arg.config:get_data(pn), arg.path, new_value)
				arg.config:set_dirty(pn)
				MESSAGEMAN:Broadcast("ConfigValueChanged", {
					config_name= arg.config.name, field_name= arg.path,
					value= new_value, pn= pn})
				return vtype_ret(new_value, arg.value_type)
			end
		end
	},
	profile= {
		arg= function(params)
			return params.name
		end,
		get= function(name, pn)
			local profile= PROFILEMAN:GetProfile(pn)
			return profile["Get"..name](profile)
		end,
		set= function(name, value, pn)
			local profile= PROFILEMAN:GetProfile(pn)
			profile["Set"..name](profile, value)
		end,
	},
	data= {
		arg= function(params)
			return {data= params.data, path= params.path}
		end,
		get= function(arg, pn)
			return get_element_by_path(arg.data, arg.path)
		end,
		set= function(arg, value, pn)
			set_element_by_path(arg.data, arg.path, value)
			MESSAGEMAN:Broadcast("DataValueChanged", {
				data_name= arg.data.name, field_name= arg.path,
				value= value, pn= pn})
		end,
	},
	player_option= {
		arg= function(params)
			return params.name
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
		end,
	},
	song_option= {
		arg= function(params)
			return params.name
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
	add_broad_type= function(name, params)
		broad_types[name]= params
	end,
	set_close_default_to_top= function(val)
		submenu_puts_closure_at_top= val
	end,
	set_submenu_close_item= function(item)
		submenu_close_item= item
	end,
	clear_submenu_close_item= function(item)
		submenu_close_item= default_close_item
	end,
	add_close_item= function(items)
		if not submenu_close_item then return end
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
			if submenu_puts_closure_at_top then
				table.insert(items, 1, default_close_item)
			else
				items[#items+1]= default_close_item
			end
		end
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
		local ret= generic_entry(
			combine_menu_params(
				params, {
					get= specific_entry.get, set= specific_entry.set, reset= reset,
					arg= specific_entry.arg(params), sub_type= sub_type}))
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
		local ret= {}
		for eid= 1, #info do
			local entry= info[eid]
			assert(type(entry) == "table", "Menu entry " .. eid .. " cannot hold drinks or food.")
			local entype= entry[1]
			local type_handlers= {
				close= function()
					return default_close_item
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
						on_open= entry.on_open, on_close= entry.on_close,
						type_hint= {main= "submenu", sub= sub_type},
					}
					return add_translation_params(ret, entry)
				end,
				action= function()
					return {name= entry[2], func= entry[3], arg= entry[4]}
				end,
				custom= function()
					return entry[2]
				end,
			}
			local handler= type_handlers[entype]
			assert(handler, "Menu entry " .. eid .. " is of unknown type.")
			local success, item= pcall(handler)
			assert(success, "Menu entry " .. eid .. " had problem..."..tostring(item))
			ret[#ret+1]= item
		end
		return ret
	end,
}
