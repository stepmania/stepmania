-- This explanation is incomplete.

local move_time= 0.1
local line_height= 24

function set_nested_options_menu_line_height(h)
	line_height= h
end
function get_nested_options_menu_line_height(h)
	return line_height
end

function add_defaults_to_params(params, defaults)
	for key, value in pairs(defaults) do
		if params[key] == nil then params[key]= value end
	end
end

function add_blank_tables_to_params(params, table_names)
	for i, name in ipairs(table_names) do
		if not params[name] then params[name]= {} end
	end
end

local function play_gain_focus(self)
	self.container:playcommand("GainFocus")
end
local function play_lose_focus(self)
	self.container:playcommand("LoseFocus")
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

-- Not everything needs to be translated, so options have a flag attached to
-- them and this holds the logic for using the flag.
local function get_string_if_translatable(translatable, section, str)
	if not section or not str or section == "" or str == "" then return "" end
	if not translatable then return str end
	if not THEME:HasString(section, str) then return str end
	return THEME:GetString(section, str)
end

local cursor_param_defaults= {
	name= "", parts_name= "OptionsCursor",
}
nesty_cursor_mt= {
	__index= {
		create_actors= function(self, params)
			add_defaults_to_params(params, cursor_param_defaults)
			self.name= params.name
			self.pn= params.pn
			local frame= Def.ActorFrame{
				Name= self.name, InitCommand= function(subself)
					self.container= subself
					self.left:horizalign(right)
					self.right:horizalign(left)
					if self.pn then
						self:diffuse(PlayerColor(self.pn))
					end
				end,
				LoadActor(THEME:GetPathG(params.parts_name, "Middle")) ..
				{InitCommand= function(subself) self.middle= subself end},
				LoadActor(THEME:GetPathG(params.parts_name, "Left")) ..
				{InitCommand= function(subself) self.left= subself end},
				LoadActor(THEME:GetPathG(params.parts_name, "Right")) ..
				{InitCommand= function(subself) self.right= subself end},
			}
			return frame;
		end,
		refit= function(self, nx, ny, nw, nh)
			nx= nx or self.container:GetX()
			ny= ny or self.container:GetY()
			nw= nw or self.w
			nh= nh or self.h
			local new_size= (self.w ~= nw) or (self.h ~= nh)
			self.x= nx
			self.y= ny
			self.w= nw or self.w
			self.h= nh or self.h
			self.container:stoptweening():linear(.1):xy(self.x, self.y)
			if new_size then
				for i, part in ipairs{self.left, self.middle, self.right} do
					part:stoptweening():linear(.1):zoomtoheight(self.h)
				end
				self.left:x(self.w*-.5)
				self.middle:zoomtowidth(self.w)
				self.right:x(self.w*.5)
			end
		end,
		diffuse= function(self, color)
			for i, part in ipairs{self.left, self.middle, self.right} do
				part:diffuse(color)
			end
		end,
		hide= function(self)
			self.hidden= true
			self.container:visible(false)
		end,
		unhide= function(self)
			self.hidden= false
			self.container:visible(true)
		end,
}}

local simple_item_default_params= {
	name= "", diffuse= {1, 1, 1, 1}, stroke= {0, 0, 0, 0},
}
nesty_option_item_default_set_command= function(self, params)
	self:settext(params.text)
	width_limit_text(self, params.width, params.zoom)
	params.ret_width= self:GetZoomedWidth()
end
local function default_item_text()
	return Def.BitmapText{
		Font= "Common Normal", SetCommand= nesty_option_item_default_set_command}
end
local simple_item_mt= {
	__index= {
		create_actors= function(self, params)
			add_defaults_to_params(params, option_item_default_params)
			self.name= name
			self.zoom= 1
			self.width= SCREEN_WIDTH
			self.prev_index= 1
			self.text_width= 0
			self.translation_section= "OptionNames"
			self.underline= setmetatable({}, nesty_cursor_mt)
			local text_actor= params.text_actor or default_item_text()
			text_actor.Name= "text"
			return Def.ActorFrame{
				Name= name, InitCommand= function(subself)
					self.text= subself:GetChild("text")
					self.text:zoom(self.zoom):diffuse(params.diffuse)
					if self.text.strokecolor then
						self.text:strokecolor(params.stroke)
					end
					self.container= subself
					self:lose_focus()
				end,
				self.underline:create_actors{
					name= "underline", parts_name= "OptionsUnderline"},
				text_actor,
			}
		end,
		set_geo= function(self, width, height, zoom)
			self.width= width
			self.zoom= zoom
			self.height= height
			self.text:zoom(zoom)
			self.underline:refit(nil, height/2, width, height/4)
			self.underline.container:finishtweening()
		end,
		set_player_number= function(self, pn)
			self.pn= pn
			self.underline:diffuse(PlayerColor(pn))
		end,
		transform= function(self, item_index, num_items, is_focus)
			local changing_edge= math.abs(item_index-self.prev_index)>num_items/2
			if changing_edge then
				self.container:diffusealpha(0)
			end
			self.container:finishtweening():linear(move_time)
				:xy(0, (item_index-1) * (self.height or self.text:GetZoomedHeight()))
				:linear(move_time):diffusealpha(1)
			self.prev_index= item_index
		end,
		set= function(self, info)
			self.info= info
			if info then
				self:set_text(info.text)
				if type(info.value) == "bool" then
					self:set_underline(info.value)
				else
					self:set_underline(false)
				end
			else
				self.text:playcommand("Set", {text= "", width= self.width, zoom= self.zoom})
				self.text_width= 0
				self:set_underline(false)
			end
		end,
		set_underline= function(self, u)
			if u then
				self.underline.container:stoptweening():accelerate(0.25):zoom(1)
			else
				self.underline.container:stoptweening():decelerate(0.25):zoom(0)
			end
		end,
		set_text= function(self, t)
			local text_params= {text= get_string_if_translatable(
				self.info.translatable, self.translation_section, t),
				width= self.width, zoom= self.zoom, type= self.info.type,
				value= self.info.value}
			self.text:playcommand("Set", text_params)
			self.text_width= text_params.ret_width or 0
			self.underline:refit(nil, nil, self.text_width, nil)
		end,
		get_cursor_fit= function(self)
			local ret= {0, 0, 0, self.height + 4}
			if self.text_width > 0 then
				ret[3]= self.text_width + 4
			end
			return ret
		end,
		gain_focus= play_gain_focus,
		lose_focus= play_lose_focus,
}}

local value_item_default_params= {
	name= "", text_font= "Common Normal", text_width= .70,
	value_font= "Common Normal", value_width= .25,
}
local value_item_mt= {
	__index= {
		create_actors= function(self, params)
			add_defaults_to_params(params, value_item_default_params)
			self.name= name
			self.prev_index= 1
			self.translation_section= "OptionNames"
			self.text_width= params.text_width
			self.value_width= params.value_width
			self.type_images= params.type_images or {}
			local frame= Def.ActorFrame{
				InitCommand= function(subself)
					self.container= subself
					self.text= subself:GetChild("text"):horizalign(left)
					self.value_text= subself:GetChild("vtext"):horizalign(right)
					self.value_image= subself:GetChild("vimage"):horizalign(right):animate(false)
				end,
			}
			local parts= {{
					thing= Def.BitmapText{Name= "text"},
					commands= params.text_commands or {}
										},{
					thing= Def.BitmapText{Name= "vtext"},
					commands= params.value_text_commands or {}
											},{
					thing= Def.Sprite{Name= "vimage"},
					commands= params.value_image_commands or {}
			}}
			for i, part in ipairs(parts) do
				for name, func in pairs(part.commands) do
					if name ~= "Name" then
						part.thing[name]= func
					end
				end
				frame[#frame+1]= part.thing
			end
			return frame
		end,
		set_geo= function(self, width, height, zoom)
			self.width= width
			self.height= height
			self.zoom= zoom
			self.text:zoom(zoom)
			self.value_text:zoom(zoom)
			self:refit_parts()
		end,
		refit_parts= function(self)
			local text_width= self.width * self.text_width
			local text_left= self.width * -.5
			width_limit_text(self.text, text_width, self.zoom)
			self.text:x(text_left)
			local value_width= self.width * self.value_width
			local value_right= self.width * .5
			width_limit_text(self.value_text, value_width, self.zoom)
			self.value_text:x(value_right)
			scale_to_fit(self.value_image, value_width, self.height)
			self.value_image:x(value_right)
		end,
		set_player_number= function(self, pn)
			self.pn= pn
		end,
		transform= function(self, item_index, num_items, is_focus)
			local changing_edge= math.abs(item_index-self.prev_index)>num_items/2
			if changing_edge then
				self.container:diffusealpha(0)
			end
			self.container:finishtweening():linear(move_time)
				:xy(0, (item_index-1) * (self.height or self.text:GetZoomedHeight()))
				:linear(move_time):diffusealpha(1)
			self.prev_index= item_index
		end,
		set_value_image= function(self, path)
			self.value_image:visible(true):Load(path)
			self.value_text:visible(false):settext("")
			self.using_image= true
			if self.info.type == "bool" then
				if self.info.value then
					self.value_image:setstate(1)
				else
					self.value_image:setstate(0)
				end
			elseif self.info.type == "choice" then
				if self.info.value then
					self.value_image:setstate(1):visible(true)
				else
					self.value_image:visible(false)
				end
			else
				self.value_image:setstate(0)
			end
		end,
		set_value_text= function(self, text)
			self.value_image:visible(false)
			self.value_text:visible(true):settext(text)
			self.using_image= false
		end,
		set= function(self, info)
			self.info= info
			if info then
				local text= get_string_if_translatable(
					info.translatable, self.translation_section, info.text)
				self.text:settext(text)
				local use_value_image= false
				if self.type_images[info.type] then
					use_value_image= true
				end
				if info.type == "menu" and info.value ~= nil then
					use_value_image= false
				end
				if use_value_image then
					self:set_value_image(self.type_images[info.type])
				else
					if info.value ~= nil then
						local val_text= tostring(info.value)
						if type(info.value) == "string" and THEME:HasString(
							self.translation_section, info.value) then
							val_text= THEME:GetString(self.translation_section, info.value)
						end
						self:set_value_text(val_text)
					else
						self:set_value_text("")
					end
				end
				self:refit_parts()
			else
				self.text:settext("")
				self:set_value_text("")
			end
		end,
		get_cursor_fit= function(self)
			return {0, 0, self.width + 4, self.height + 4}
		end,
		gain_focus= play_gain_focus,
		lose_focus= play_lose_focus,
}}

nesty_items= {
	simple= simple_item_mt,
	value= value_item_mt,
}

local option_display_default_params= {
	name= "", x= 0, y= 0, height= 0, el_width= 0, el_height= 0, el_zoom= 1,
	no_heading= false, no_status= false, item_mt= nesty_items.simple,
}
nesty_option_display_mt= {
	__index= {
		create_actors= function(self, params)
			add_defaults_to_params(params, option_display_default_params)
			add_blank_tables_to_params(params, {"item_params"})
			params.heading_zoom= params.heading_zoom or params.el_zoom
			params.status_zoom= params.status_zoom or params.el_zoom
			params.heading_height= params.heading_height or params.el_height
			params.status_height= params.status_height or params.el_height
			self.heading_zoom= params.heading_zoom
			self.status_zoom= params.status_zoom
			local el_count= 1
			local elements_height= params.height
			local el_start= 0
			if not params.no_heading then
				elements_height= elements_height - params.heading_height
				el_start= el_start + params.heading_height
			end
			if not params.no_status then
				elements_height= elements_height - params.status_height
				el_start= el_start + params.status_height
			end
			if (not params.no_heading) or (not params.no_status) then
				elements_height= elements_height - (params.el_height * .5)
				el_start= el_start + (params.el_height * .5)
			end
			self.name= params.name
			self.el_width= params.el_width or SCREEN_WIDTH
			self.el_height= params.el_height or line_height
			el_count= math.floor(elements_height / self.el_height)
			self.el_zoom= params.el_zoom or 1
			self.translation_section= "OptionNames"
			local frame= {
				Name= name, InitCommand= function(subself)
					subself:xy(params.x, params.y)
					self.container= subself
					self.heading= subself:GetChild("heading")
					local next_y= 0
					if self.heading then
						self.heading:zoom(params.heading_zoom)
						next_y= next_y + params.heading_height
					end
					self.status= subself:GetChild("status")
					if self.status then
						self.status:zoom(params.status_zoom):y(next_y)
					end
					self:regeo_items()
				end,
				OnCommand= params.on or noop_nil
			}
			if not params.no_heading then
				local head_actor= params.heading_actor or default_item_text()
				head_actor.Name= "heading"
				frame[#frame+1]= head_actor
			end
			if not params.no_status then
				local disp_actor= params.status_actor or default_item_text()
				disp_actor.Name= "status"
				frame[#frame+1]= disp_actor
			end
			self.scroller= setmetatable({disable_wrapping= true}, item_scroller_mt)
			self.item_mt= params.item_mt
			frame[#frame+1]= self.scroller:create_actors(
				"wheel", el_count, params.item_mt, 0, el_start, params.item_params)
			return Def.ActorFrame(frame)
		end,
		set_player_number= function(self, pn)
			for i, item in ipairs(self.scroller.items) do
				item:set_player_number(pn)
			end
		end,
		set_translation_section= function(self, section)
			self.translation_section= section
			for i= 1, #self.scroller.items do
				self.scroller.items[i].translation_section= section
			end
		end,
		set_el_geo= function(self, width, height, zoom)
			self.el_width= width or self.el_width
			self.el_height= height or self.el_height
			self.el_zoom= zoom or self.el_zoom
			self:regeo_items()
		end,
		regeo_items= function(self)
			for i, item in ipairs(self.scroller.items) do
				item:set_geo(self.el_width, self.el_height, self.el_zoom, self.square_size)
			end
		end,
		set_heading= function(self, h)
			if self.heading then
				self.heading:playcommand("Set", {text= get_string_if_translatable(
					true, self.translation_section, h), width= self.el_width, zoom= self.heading_zoom})
			end
		end,
		set_status= function(self, d)
			if self.status then
				self.status:playcommand("Set", {text= get_string_if_translatable(
					true, self.translation_section, d), width= self.el_width, zoom= self.status_zoom})
			end
		end,
		set_info_set= function(self, info, pos)
			self.scroller:set_info_set(info, pos or 1)
			self:unhide()
		end,
		set_element_info= function(self, element, info)
			self.scroller:set_element_info(element, info)
		end,
		get_element= function(self, element)
			return self.scroller:get_items_by_info_index(element)[1]
		end,
		scroll= function(self, pos)
			self.scroller:scroll_to_pos(pos)
		end,
		hide= function(self) self.hidden= true self.container:diffusealpha(0) end,
		unhide= function(self) self.hidden= false self.container:diffusealpha(1) end,
		lose_focus_items= function(self)
			for i, item in ipairs(self.scroller.items) do
				item:lose_focus()
			end
		end,
}}

nesty_menu_up_element= {text= "&leftarrow;", type= "back"}

local general_menu_mt= {
	__index= {
		set_player_info= function(self, pn)
			self.pn= pn
		end,
		set_display= function(self, display)
			self.display= display
			display:set_info_set(self.info_set, self.cursor_pos)
			self:set_status()
		end,
		set_status= function() end, -- This is meant to be overridden.
		can_exit= function(self)
			return self.cursor_pos == 1
		end,
		get_cursor_element= function(self)
			if self.display then
				return self.display:get_element(self.cursor_pos)
			else
				lua.ReportScriptError("menu has no display to fetch cursor element from.")
				return nil
			end
		end,
		update_el_text= function(self, pos, text)
			self.info_set[pos].text= text
			if self.display then
				self.display:set_element_info(pos, self.info_set[pos])
			end
		end,
		update_el_value= function(self, pos, value)
			self.info_set[pos].value= value
			if self.display then
				self.display:set_element_info(pos, self.info_set[pos])
			end
		end,
		scroll_to_pos= function(self, pos)
			self.cursor_pos= ((pos-1) % #self.info_set) + 1
			self.display:scroll(self.cursor_pos)
		end,
		interpret_code= function(self, code)
			-- Protect against other code changing cursor_pos to an element that
			-- isn't visible.
			local function unfocus_cursor(self)
				local prev_el= self:get_cursor_element()
				if prev_el then
					prev_el:lose_focus()
				end
			end
			local funs= {
				MenuLeft= function(self)
					unfocus_cursor(self)
					if self.cursor_pos > 1 then
						self.cursor_pos= self.cursor_pos - 1
					else
						self.cursor_pos= #self.info_set
					end
					self.display:scroll(self.cursor_pos)
					self:get_cursor_element():gain_focus()
					return true, false, "move_up"
				end,
				MenuRight= function(self)
					unfocus_cursor(self)
					if self.cursor_pos < #self.info_set then
						self.cursor_pos= self.cursor_pos + 1
					else
						self.cursor_pos= 1
					end
					self.display:scroll(self.cursor_pos)
					self:get_cursor_element():gain_focus()
					return true, false, "move_down"
				end,
				Start= function(self)
					if self.info_set[self.cursor_pos].text == nesty_menu_up_element.text then
						-- This position is the "up" element that moves the
						-- cursor back up the options tree.
						return "pop", false, "pop"
					end
					if self.interpret_start then
						local action, action_data= self:interpret_start()
						local sound= self.info_set[self.cursor_pos].sound or "act"
						if self.scroll_to_move_on_start then
							local pos_diff= 1 - self.cursor_pos
							self.cursor_pos= 1
							self.display:scroll(self.cursor_pos)
						end
						return action, action_data, sound
					end
				end,
				Select= function(self)
					self.cursor_pos= 1
					self.display:scroll(self.cursor_pos)
					return true, false, "move"
				end,
				Back= function(self)
					return "pop", false, "pop"
				end,
			}
			funs.MenuUp= funs.MenuLeft
			funs.MenuDown= funs.MenuRight
			if funs[code] then return funs[code](self) end
			return false
		end,
		get_item= function(self)
			return self.info_set[self.cursor_pos]
		end,
		get_item_name= function(self)
			local item= self.info_set[self.cursor_pos]
			if item then return item.name or item.text end
		end,
}}

nesty_option_menus= {}

-- MENU ENTRIES STRUCTURE
-- {}
--   name= string -- Name for the entry
--   args= {} -- Args to return to options_menu_mt to construct the new menu
--     menu= {} -- metatable for the submenu
--     args= {} -- extra args for the initialize function of the metatable
nesty_option_menus.menu= {
	type= "menu",
	__index= {
		initialize= function(self, pn, initializer_args, no_up, up_text)
			self.init_args= initializer_args
			self.pn= pn
			self.no_up= no_up
			self.up_text= up_text
			self:recall_init()
		end,
		insert_up= function(self, menu_data)
			if not self.no_up then
				if self.up_text then
					for i= 1, #menu_data do
						if menu_data[i].text == self.up_text then return end
					end
					table.insert(menu_data, 1, {text= self.up_text, translatable= true, type= "back"})
				else
					for i= 1, #menu_data do
						if menu_data[i].text == nesty_menu_up_element.text then return end
					end
					table.insert(menu_data, 1, nesty_menu_up_element)
				end
			end
		end,
		recall_init= function(self)
			if type(self.init_args) == "function" then
				self.menu_data= self.init_args(self.pn)
			else
				self.menu_data= self.init_args
			end
			self:insert_up(self.menu_data)
			self.name= self.menu_data.name or ""
			self.recall_init_on_pop= self.menu_data.recall_init_on_pop
			self.special_handler= self.menu_data.special_handler
			self.destructor= self.menu_data.destructor
			self:set_status()
			self:reset_info()
		end,
		reset_info= function(self)
			local old_option_name= ""
			if self.cursor_pos then
				old_option_name= self.info_set[self.cursor_pos].text
			end
			self.info_set= {}
			self.shown_data= {}
			self.cursor_pos= 1
			self:update_info(self.menu_data)
			if old_option_name ~= "" then
				for pos= 1, #self.info_set do
					if self.info_set[pos].text == old_option_name then
						self.cursor_pos= pos
					end
				end
			end
			if self.display and self.display.scroller then
				self.display:set_info_set(self.info_set)
				self:scroll_to_pos(self.cursor_pos)
				for i, item in ipairs(self.display.scroller.items) do
					item.container:finishtweening()
				end
			end
		end,
		update_info= function(self, new_menu_data)
			local next_shown= 1
			for i, data in ipairs(new_menu_data) do
				local show= true
				if data.req_func then
					show= show and data.req_func(self.pn)
				end
				if show then
					local disp_slot= next_shown
					self.shown_data[next_shown]= data
					local disp_text= data.text or data.name
					if not self.info_set[disp_slot] then
						self.info_set[disp_slot]= {}
					end
					if data.menu then
						self.info_set[disp_slot].type= data.type or data.menu.type
					else
						self.info_set[disp_slot].type= data.type
					end
					self.info_set[disp_slot].text= disp_text
					if type(data.value) == "function" then
						self.info_set[disp_slot].value= data.value(self.pn)
					else
						self.info_set[disp_slot].value= data.value
					end
					self.info_set[disp_slot].translatable= data.translatable
					self.info_set[disp_slot].translatable_value= data.translatable_value
					if data.args and type(data.args) == "table" then
						data.args.name= data.name
					end
					if self.display then
						self.display:set_element_info(
							disp_slot, self.info_set[disp_slot])
					end
					next_shown= next_shown + 1
				end
			end
			while #self.shown_data >= next_shown do
				local index= #self.shown_data
				self.shown_data[index]= nil
				self.info_set[index]= nil
				if self.display then
					self.display:set_element_info(index, nil)
				end
			end
			self.menu_data= new_menu_data
		end,
		set_status= function(self)
			if self.display then
				self.display:set_heading(self.name or "")
				self.display:set_status(self.menu_data.status or "")
			end
		end,
		update= function(self)
			if GAMESTATE:IsPlayerEnabled(self.pn) then
				self.display:unhide()
			else
				self.display:hide()
			end
		end,
		interpret_start= function(self)
			local data= self.shown_data[self.cursor_pos]
			if self.special_handler then
				local handler_ret= {self.special_handler(self, data)}
				if handler_ret[1] then
					if handler_ret[1] == "recall_init" then
						self:recall_init()
						return true
					else
						return unpack(handler_ret)
					end
				end
				return true
			else
				if data then
					if self.up_text and data.text == self.up_text then
						return "pop"
					else
						return "push", data
					end
				else
					return false
				end
			end
		end,
		get_item= function(self, pos)
			pos= (pos or self.cursor_pos)
			if pos == 0 then
				return self.info_set[1]
			end
			return self.shown_data[pos]
		end,
		get_item_name= function(self, pos)
			pos= (pos or self.cursor_pos)
			local shown= self.shown_data[pos]
			if shown then
				return shown.name or shown.text
			end
			return self.up_text or ""
		end,
}}

local function find_scale_for_number(num, min_scale)
	local cv= math.round(num /10^min_scale) * 10^min_scale
	local prec= math.max(0, -min_scale)
	local cs= ("%." .. prec .. "f"):format(cv)
	local ret_scale= 0
	for n= 1, #cs do
		if cs:sub(-n, -n) ~= "0" then
			ret_scale= math.min(ret_scale, min_scale + (n-1))
		end
	end
	return ret_scale, cv
end

nesty_option_menus.adjustable_float= {
	type= "number",
	__index= {
		initialize= function(self, pn, extra)
			local function check_member(member_name)
				assert(self[member_name],
							 "adjustable_float '" .. self.name .. "' warning: " ..
								 member_name .. " not provided.")
			end
			local function to_text_default(pn, value)
				if value == -0 then return "0" end
				return tostring(value)
			end
			--Trace("adjustable_float extra:")
			--rec_print_table(extra)
			assert(extra, "adjustable_float passed a nil extra table.")
			self.name= extra.name
			self.cursor_pos= 1
			self.pn= pn
			self.reset_value= extra.reset_value or 0
			self.min_scale= extra.min_scale
			check_member("min_scale")
			self.scale= extra.scale or 0
			self.current_value= extra.initial_value(pn) or 0
			if self.current_value ~= 0 then
				self.min_scale_used, self.current_value=
					find_scale_for_number(self.current_value, self.min_scale)
			end
			self.min_scale_used= math.min(self.scale, self.min_scale_used or 0)
			self.max_scale= extra.max_scale
			check_member("max_scale")
			if self.min_scale > self.max_scale then
				self.min_scale, self.max_scale= self.max_scale, self.min_scale
			end
			self.set= extra.set
			check_member("set")
			self.val_min= extra.val_min
			self.val_max= extra.val_max
			self.val_to_text= extra.val_to_text or to_text_default
			self.scale_to_text= extra.scale_to_text or to_text_default
			self.info_set= {nesty_menu_up_element}
			self.menu_functions= {function() return "pop" end}
			--local scale_text= THEME:GetString("OptionNames", "scale")
			local scale_text= "scale"
			local scale_range= self.max_scale - self.min_scale
			if scale_range < 4 then
				-- {+100, +10, +1, -1, -10, -100, Round, Reset}
				self.mode= "small"
				for s= self.max_scale, self.min_scale, -1 do
					self.info_set[#self.info_set+1]= {
						text= "+"..self.scale_to_text(self.pn, 10^s), sound= "inc",
						type= "action"}
					self.menu_functions[#self.menu_functions+1]= function()
						self.min_scale_used= math.min(s, self.min_scale_used)
						self:set_new_val(self.current_value + 10^s)
						return true
					end
					self.info_set[#self.info_set+1]= {
						text= "-"..self.scale_to_text(self.pn, 10^s), sound= "dec",
						type= "action"}
					self.menu_functions[#self.menu_functions+1]= function()
						self.min_scale_used= math.min(s, self.min_scale_used)
						self:set_new_val(self.current_value - 10^s)
						return true
					end
				end
			else
				-- {+scale, -scale, ...}
				self.info_set[#self.info_set+1]= {text= "+"..self.scale_to_text(self.pn, 10^self.scale),sound= "inc", type= "action"}
				self.info_set[#self.info_set+1]= {text= "-"..self.scale_to_text(self.pn, 10^self.scale),sound= "dec", type= "action"}
				self.menu_functions[#self.menu_functions+1]= function() -- increment
					self:set_new_val(self.current_value + 10^self.scale)
					return true
				end
				self.menu_functions[#self.menu_functions+1]= function() -- decrement
					self:set_new_val(self.current_value - 10^self.scale)
					return true
				end
				if scale_range < 7 then
					-- {..., scale=1, scale=2, ..., Round, Reset}
					self.mode= "medium"
					for s= self.min_scale, self.max_scale do
						self.info_set[#self.info_set+1]= {
							text= scale_text.."="..(10^s), sound= "inc", typej= "action"}
						self.menu_functions[#self.menu_functions+1]= function()
							self:set_new_scale(s)
						end
					end
				else
					-- {..., scale*10, scale/10, Round, Reset}
					self.mode= "bignum"
					self.info_set[#self.info_set+1]= {text= scale_text.."*10",sound= "inc", type= "action"}
					self.info_set[#self.info_set+1]= {text= scale_text.."/10",sound= "dec", type= "action"}
					self.menu_functions[#self.menu_functions+1]= function() -- scale up
						self:set_new_scale(self.scale + 1)
						return true
					end
					self.menu_functions[#self.menu_functions+1]= function() -- scale down
						self:set_new_scale(self.scale - 1)
						return true
					end
				end
			end
			if self.min_scale < 0 then
				self.info_set[#self.info_set+1]= {
					text= "Round", translatable= true, type= "action"}
				self.menu_functions[#self.menu_functions+1]= function()
					self:set_new_val(math.round(self.current_value))
					return true
				end
			end
			self.info_set[#self.info_set+1]= {
				text= "Reset", translatable= true, type= "action"}
			self.menu_functions[#self.menu_functions+1]= function()
				local new_scale, new_value=
					find_scale_for_number(self.reset_value, self.min_scale)
				if self.mode ~= "small" then
					self:set_new_scale(new_scale)
				end
				self:set_new_val(new_value)
				return true
			end
		end,
		interpret_start= function(self)
			if self.menu_functions[self.cursor_pos] then
				return self.menu_functions[self.cursor_pos]()
			end
			return false
		end,
		set_status= function(self)
			if self.display then
				self.display:set_heading(self.name)
				local val_text=
					self.val_to_text(self.pn, self.current_value)
				self.display:set_status(val_text)
			end
		end,
		cooked_val= function(self, nval)
			return nval
		end,
		set_new_val= function(self, nval)
			local raise= 10^-self.min_scale_used
			local lower= 10^self.min_scale_used
			local rounded_val= math.round(nval * raise) * lower
			if self.val_max and rounded_val > self.val_max then
				rounded_val= self.val_max
			end
			if self.val_min and rounded_val < self.val_min then
				rounded_val= self.val_min
			end
			self.current_value= rounded_val
			rounded_val= self:cooked_val(rounded_val)
			self.set(self.pn, rounded_val)
			self:set_status()
		end,
		set_new_scale= function(self, nscale)
			if nscale >= self.min_scale and nscale <= self.max_scale then
				self.min_scale_used= math.min(nscale, self.min_scale_used)
				self.scale= nscale
				self:update_el_text(2, "+" .. self.scale_to_text(self.pn, 10^nscale))
				self:update_el_text(3, "-" .. self.scale_to_text(self.pn, 10^nscale))
			end
		end
}}

nesty_option_menus.enum_option= {
	type= "enum",
	__index= {
		initialize= function(self, pn, extra)
			self.name= extra.name
			self.pn= pn
			self.enum_vals= {}
			self.info_set= {nesty_menu_up_element}
			self.cursor_pos= 1
			self.get= extra.get
			self.set= extra.set
			self.fake_enum= extra.fake_enum
			self.ops_obj= extra.obj_get(pn)
			local cv= self:get_val()
			for i, v in ipairs(extra.enum) do
				self.enum_vals[#self.enum_vals+1]= v
				self.info_set[#self.info_set+1]= {
					text= self:short_string(v), translatable= true, value= (v == cv), type= "choice"}
			end
		end,
		short_string= function(self, val)
			if self.fake_enum then
				return val
			else
				return ToEnumShortString(val)
			end
		end,
		interpret_start= function(self)
			if self.cursor_pos > 1 then
				for i, info in ipairs(self.info_set) do
					if info.value then
						self:update_el_value(i, false)
					end
				end
				self:update_el_value(self.cursor_pos, true)
				if self.ops_obj then
					self.set(self.pn, self.ops_obj, self.enum_vals[self.cursor_pos-1])
				else
					self.set(self.pn, self.enum_vals[self.cursor_pos-1])
				end
				self.display:set_status(self:short_string(self:get_val()))
				return true, false
			else
				return false
			end
		end,
		get_val= function(self)
			if self.ops_obj then
				return self.get(self.pn, self.ops_obj)
			else
				return self.get(self.pn)
			end
		end,
		set_status= function(self)
			self.display:set_heading(self.name)
			self.display:set_status(self:short_string(self:get_val()))
		end
}}

function set_nesty_option_metatables()
	for k, set in pairs(nesty_option_menus) do
		setmetatable(set.__index, general_menu_mt)
	end
end
set_nesty_option_metatables()

-- This exists to hand to menus that pass out of view but still exist.
local fake_display= {is_fake= true}
for k, v in pairs(nesty_option_display_mt.__index) do
	fake_display[k]= function() end
end

local menu_stack_param_defaults= {
	x= 0, y= 0, width= _screen.w, num_displays= 1, el_height= line_height,
	display_mt= nesty_option_display_mt,
	cursor_above_options= false, cursor_mt= nesty_cursor_mt,
}
nesty_menu_stack_mt= {
	__index= {
		create_actors= function(self, params)
			add_defaults_to_params(params, menu_stack_param_defaults)
			add_blank_tables_to_params(params, {"display_params", "cursor_params", "menu_sounds"})
			self.el_height= params.el_height
			self.name= params.name
			self.pn= params.pn
			self.menu_stack= {}
			self.on_close_stack= {}
			local pcolor= PlayerColor(params.pn)
			local frame= {
				Name= params.name, InitCommand= function(subself)
					subself:xy(params.x, params.y)
					self.container= subself
					self.cursor:refit(nil, nil, 20, self.el_height)
					for i, disp in ipairs(self.displays) do
						disp:set_player_number(self.pn)
					end
				end
			}
			self.menu_sounds= {}
			for name, sound in pairs(params.menu_sounds) do
				frame[#frame+1]= Def.Sound{
					File= sound, IsAction= true, InitCommand= function(subself)
						self.menu_sounds[name]= subself end}
			end
			self.displays= {}
			for i= 1, params.num_displays do
				self.displays[#self.displays+1]= setmetatable({}, params.display_mt)
			end
			local sep= params.width / #self.displays
			if #self.displays == 1 then sep= 0 end
			local off= sep / 2
			self.cursor= setmetatable({}, params.cursor_mt)
			local cursor_params= params.cursor_params
			add_defaults_to_params(cursor_params, {name= "cursor", pn= params.pn})
			if not params.cursor_above_options then
				frame[#frame+1]= self.cursor:create_actors(cursor_params)
			end
			local disp_el_width_limit= (params.width / #self.displays) - 8
			add_defaults_to_params(params.display_params, {
				el_width= disp_el_width_limit, el_height= self.el_height,
				el_zoom= self.zoom, height= params.height, no_heading= false,
				no_display= false, el_height= params.el_height,
				item_mt= nesty_items.value})
			for i, disp in ipairs(self.displays) do
				local sub_params= DeepCopy(params.display_params)
				sub_params.name= "disp" .. i
				sub_params.x= off+sep * (i-1)
				sub_params.y= 0
				frame[#frame+1]= disp:create_actors(sub_params)
			end
			if params.cursor_above_options then
				frame[#frame+1]= self.cursor:create_actors(cursor_params)
			end
			if params.translation_section then
				self:set_translation_section(params.translation_section)
			end
			return Def.ActorFrame(frame)
		end,
		set_translation_section= function(self, section)
			for i, disp in ipairs(self.displays) do
				disp:set_translation_section(section)
			end
		end,
		assign_displays= function(self, start)
			local oss= self.menu_stack
			for i= #oss, 1, -1 do
				oss[i]:set_display(self.displays[start] or fake_display)
				start= start - 1
			end
		end,
		lose_focus_top_display= function(self)
			local top_display= math.min(#self.displays, #self.menu_stack)
			if self.displays[top_display] then
				self.displays[top_display]:lose_focus_items()
			end
		end,
		push_display_stack= function(self)
			local use_display= math.min(#self.displays, #self.menu_stack+1)
			self:assign_displays(use_display - 1)
			self:hide_unused_displays(use_display)
			return use_display
		end,
		pop_display_stack= function(self)
			local oss= self.menu_stack
			local use_display= math.min(#self.displays, #oss)
			self:assign_displays(use_display)
			for i= #oss, 1, -1 do
				local curr_set= oss[i]
				if not curr_set.display.is_fake then
					if curr_set.recall_init_on_pop then
						curr_set:recall_init()
					end
					curr_set:reset_info()
				end
			end
			self:hide_unused_displays(use_display)
			return use_display
		end,
		hide_unused_displays= function(self, last_used_display)
			for i= last_used_display+1, #self.displays do
				self.displays[i]:hide()
			end
		end,
		push_menu_stack= function(
				self, new_set_menu, new_set_initializer_args, base_exit, no_up)
			self:lose_focus_top_display()
			local oss= self.menu_stack
			local use_display= self:push_display_stack()
			local nos= setmetatable({}, new_set_menu)
			oss[#oss+1]= nos
			nos:set_player_info(self.pn)
			if #oss == 1 then
				nos:initialize(self.pn, new_set_initializer_args, no_up, base_exit)
			else
				nos:initialize(self.pn, new_set_initializer_args)
			end
			nos:set_display(self.displays[use_display])
			self:update_cursor_pos()
		end,
		pop_menu_stack= function(self)
			self:lose_focus_top_display()
			local oss= self.menu_stack
			local top_index= #oss
			if top_index > 0 then
				local former_top= oss[top_index]
				if former_top.destructor then former_top:destructor(self.pn) end
				if type(self.on_close_stack[top_index]) == "function" then
					self.on_close_stack[top_index](self.pn)
				end
				self.on_close_stack[top_index]= nil
				oss[top_index]= nil
				self:pop_display_stack()
			end
			self:update_cursor_pos()
		end,
		clear_menu_stack= function(self)
			while #self.menu_stack > 0 do
				self:pop_menu_stack()
			end
		end,
		enter_external_mode= function(self)
			self:hide_unused_displays(self:push_display_stack() - 1)
			self.external_thing= external_thing
		end,
		exit_external_mode= function(self)
			if self.deextern then
				self:deextern(self.pn)
				self.deextern= nil
			end
			self.external_thing= nil
			local oss= self.menu_stack
			if #oss > 0 then
				self:pop_display_stack()
			end
			self:update_cursor_pos()
		end,
		hide_disp= function(self)
			for i, disp in ipairs(self.displays) do
				disp:hide()
			end
		end,
		unhide_disp= function(self)
			for i, disp in ipairs(self.displays) do
				disp:unhide()
			end
		end,
		hide= function(self)
			self.hidden= true
			self:hide_disp()
			self.cursor:hide()
		end,
		unhide= function(self)
			self.hidden= false
			self.cursor:unhide()
		end,
		interpret_code= function(self, code)
			if self.external_thing then
				local handled, close= self.external_thing:interpret_code(code)
				if close then
					self:exit_external_mode()
				end
				return handled
			end
			local oss= self.menu_stack
			local top_set= oss[#oss]
			local action, action_data, sound= top_set:interpret_code(code)
			if action then
				if action == "pop" then
					if #oss > 1 then
						self:pop_menu_stack()
					else
						return "close"
					end
				elseif action == "push" then
					sound= action_data.sound or sound
					if action_data.execute then
						action_data.execute(self.pn)
						top_set:reset_info()
					elseif action_data.extern then
						self:enter_external_mode()
						action_data.extern(self, action_data.args, self.pn)
						self.deextern= action_data.deextern
					elseif action_data.menu then
						local nargs= action_data.args
						if action_data.exec_args and type(nargs) == "function" then
							nargs= nargs(self.pn, action_data.exec_args)
						end
						self:push_menu_stack(action_data.menu, nargs)
						if type(action_data.on_open) == "function" then
							action_data.on_open(self.pn)
						end
						self.on_close_stack[#oss]= action_data.on_close
					end
				end
				if self.menu_sounds[sound] then
					self.menu_sounds[sound]:play()
				end
			end
			self:update_cursor_pos()
			if action then return true end
			return false
		end,
		update_cursor_pos= function(self)
			local tos= self.menu_stack[#self.menu_stack]
			if not tos then return end
			local item= tos:get_cursor_element()
			if item then
				item:gain_focus()
				local fit= item:get_cursor_fit()
				local xp, yp= rec_calc_actor_pos(item.container)
				local xs, ys= rec_calc_actor_pos(self.container)
				xp= xp - xs
				yp= yp - ys
				self.cursor:refit(xp, yp, fit[3], fit[4])
			end
		end,
		refit_cursor= function(self, fit)
			self.cursor:refit(fit[1], fit[2], fit[3], fit[4])
		end,
		can_exit_screen= function(self)
			local oss= self.menu_stack
			local top_set= oss[#oss]
			return #oss <= 1 and (not top_set or top_set:can_exit())
		end,
		top_menu= function(self)
			return self.menu_stack[#self.menu_stack]
		end,
		get_cursor_item= function(self)
			local top_set= self.menu_stack[#self.menu_stack]
			if top_set.get_item then
				return top_set:get_item()
			end
			return nil
		end,
		get_cursor_item_name= function(self)
			local top_set= self.menu_stack[#self.menu_stack]
			if top_set.get_item_name then
				return top_set:get_item_name()
			end
			return ""
		end,
}}

function menu_stack_generic_input(per_player_menus, event, close_menu_callback)
	local pn= event.PlayerNumber
	if not pn then return end
	if not per_player_menus[pn] then return end
	if event.type == "InputEventType_Release" then return end
	local button= event.GameButton
	local menu_action= per_player_menus[pn]:interpret_code(button)
	if menu_action == "close" then
		if close_menu_callback then close_menu_callback(pn) end
		return true
	end
end

local function float_toggle_val_toggle_logic(old_val, on_val, off_val)
	local mid_val= (on_val + off_val) * .5
	if on_val > off_val then
		if old_val > mid_val then return off_val end
		return on_val
	else
		if old_val < mid_val then return off_val end
		return on_val
	end
end

local function float_toggle_val_bool_logic(old_val, on_val, off_val)
	local mid_val= (on_val + off_val) * .5
	if on_val > off_val then
		return old_val > mid_val
	else
		return old_val < mid_val
	end
end

local function pops_get(pn)
	return GAMESTATE:GetPlayerState(pn):get_player_options_no_defect(
		"ModsLevel_Preferred")
end

local function float_val_func(min_scale, get)
	if min_scale < 0 then
		return function(pn)
			return ("%."..(-min_scale).."f"):format(get(pn))
		end
	else
		return get
	end
end

nesty_options= {
	submenu= function(name, items)
		local ret= {
			name= name, translatable= true, menu= nesty_option_menus.menu, args= items,
		}
		return setmetatable(ret, mergable_table_mt)
	end,
	float_pref_val= function(valname, min_scale, scale, max_scale, val_min, val_max, val_reset)
		local ret= {
			name= valname, translatable= true,
			menu= nesty_option_menus.adjustable_float,
			args= {
				name= valname, min_scale= min_scale, scale= scale,
				max_scale= max_scale, val_min= val_min, val_max= val_max,
				reset_value= val_reset,
				initial_value= function()
					return PREFSMAN:GetPreference(valname)
				end,
				set= function(pn, value)
					PREFSMAN:SetPreference(valname, value)
				end,
		}}
		ret.value= float_val_func(min_scale, ret.args.initial_value)
		return setmetatable(ret, mergable_table_mt)
	end,
	bool_pref_val= function(valname)
		local ret= {
			type= "bool", name= valname, translatable= true, execute= function()
				local old_val= PREFSMAN:GetPreference(valname)
				PREFSMAN:SetPreference(valname, not old_val)
			end,
			value= function()
				return PREFSMAN:GetPreference(valname)
			end,
		}
		return setmetatable(ret, mergable_table_mt)
	end,
	float_song_mod_val= function(valname, min_scale, scale, max_scale, val_min, val_max, val_reset)
		local ret= {
			name= valname, translatable= true,
			menu= nesty_option_menus.adjustable_float,
			args= {
				name= valname, min_scale= min_scale, scale= scale,
				max_scale= max_scale, val_min= val_min, val_max= val_max,
				reset_value= val_reset,
				initial_value= function()
					local song_ops= GAMESTATE:GetSongOptionsObject("ModsLevel_Preferred")
					if not song_ops[valname] then
						lua.ReportScriptError("No such song option: " .. tostring(valname))
					end
					return song_ops[valname](song_ops)
				end,
				set= function(pn, value)
					local song_ops= GAMESTATE:GetSongOptionsObject("ModsLevel_Preferred")
					song_ops[valname](song_ops, value)
					-- Apply the change to the current and song levels too, so that if
					-- this occurs during gameplay, it takes effect immediately.
					GAMESTATE:ApplyPreferredSongOptionsToOtherLevels()
				end,
		}}
		ret.value= float_val_func(min_scale, ret.args.initial_value)
		return setmetatable(ret, mergable_table_mt)
	end,
	float_song_mod_toggle_val= function(valname, on_val, off_val)
		local ret= {
			type= "bool",
			name= valname, translatable= true, execute= function()
				local song_ops= GAMESTATE:GetSongOptionsObject("ModsLevel_Preferred")
				local old_val= song_ops[valname](song_ops)
				local new_val= float_toggle_val_toggle_logic(old_val, on_val, off_val)
				song_ops[valname](song_ops, new_val)
				-- Apply the change to the current and song levels too, so that if
				-- this occurs during gameplay, it takes effect immediately.
				GAMESTATE:ApplyPreferredSongOptionsToOtherLevels()
			end,
			value= function()
				local song_ops= GAMESTATE:GetSongOptionsObject("ModsLevel_Preferred")
				if not song_ops[valname] then
					lua.ReportScriptError("No such song option: " .. tostring(valname))
				end
				local old_val= song_ops[valname](song_ops)
				return float_toggle_val_bool_logic(old_val, on_val, off_val)
			end,
		}
		return setmetatable(ret, mergable_table_mt)
	end,
	bool_song_mod_val= function(valname)
		local ret= {
			type= "bool",
			name= valname, translatable= true, execute= function(pn)
				local song_ops= GAMESTATE:GetSongOptionsObject("ModsLevel_Preferred")
				local old_val= song_ops[valname](song_ops)
				local new_val= not old_val
				song_ops[valname](song_ops, new_val)
				-- Apply the change to the current and song levels too, so that if
				-- this occurs during gameplay, it takes effect immediately.
				GAMESTATE:ApplyPreferredSongOptionsToOtherLevels()
			end,
			value= function()
				local song_ops= GAMESTATE:GetSongOptionsObject("ModsLevel_Preferred")
				if not song_ops[valname] then
					lua.ReportScriptError("No such song option: " .. tostring(valname))
				end
				return song_ops[valname](song_ops)
			end,
		}
		return setmetatable(ret, mergable_table_mt)
	end,
	enum_player_mod_val= function(valname, enum, func_name)
		local function get(pn, obj)
			if not obj[func_name] then
				lua.ReportScriptError("No such player option: " .. tostring(func_name))
			end
			return obj[func_name](obj)
		end
		local function set(pn, obj, val)
		-- We need to inform GameState if we set the fail type so it doesn't
		-- override it with the beginner/easy preferences.
			if func_name == "FailSetting" then
				GAMESTATE:SetFailTypeExplicitlySet()
			end
			obj[func_name](obj, val)
		end
		local ret= {
			name= valname, menu= nesty_option_menus.enum_option, translatable= true,
			args= {obj_get= pops_get, get= get, set= set, enum= enum},
			value= function(pn) return get(pn, pops_get(pn)) end}
		return setmetatable(ret, mergable_table_mt)
	end,
	enum_player_mod_single_val= function(valname, value, func_name)
		local ret= {
			type= "choice",
			name= valname, translatable= true, execute= function(pn)
				local pops= pops_get(pn)
				pops[func_name](pops, value)
			end,
			value= function(pn)
				local pops= pops_get(pn)
				if not pops[func_name] then
					lua.ReportScriptError("No such player option: " .. tostring(func_name))
				end
				return pops[func_name](pops) == value
			end,
		}
		return setmetatable(ret, mergable_table_mt)
	end,
	float_player_mod_val= function(valname, min_scale, scale, max_scale, val_min, val_max, val_reset)
		local ret= {
			name= valname, translatable= true,
			menu= nesty_option_menus.adjustable_float, args= {
				name= valname, min_scale= min_scale, scale= scale,
				max_scale= max_scale, val_min= val_min, val_max= val_max,
				reset_value= val_reset,
				initial_value= function(pn)
					local plops= GAMESTATE:GetPlayerState(pn):get_player_options_no_defect("ModsLevel_Preferred")
					if not plops[valname] then
						lua.ReportScriptError("No such player option: " .. tostring(valname))
					end
					return plops[valname](plops)
				end,
				set= function(pn, value)
					local pstate= GAMESTATE:GetPlayerState(pn)
					local plops= pstate:get_player_options_no_defect("ModsLevel_Preferred")
					plops[valname](plops, value)
					pstate:ApplyPreferredOptionsToOtherLevels()
				end,
		}}
		ret.value= float_val_func(min_scale, ret.args.initial_value)
		return setmetatable(ret, mergable_table_mt)
	end,
	bool_player_mod_val= function(valname)
		local ret= {
			type= "bool",
			name= valname, translatable= true, execute= function(pn)
				local plops= GAMESTATE:GetPlayerState(pn):get_player_options_no_defect("ModsLevel_Preferred")
				local new_val= not plops[valname](plops)
				plops[valname](plops, new_val)
			end,
			value= function(pn)
				local plops= GAMESTATE:GetPlayerState(pn):get_player_options_no_defect("ModsLevel_Preferred")
				if not plops[valname] then
					lua.ReportScriptError("No such player option: " .. tostring(valname))
				end
				return plops[valname](plops)
			end,
		}
		return setmetatable(ret, mergable_table_mt)
	end,
	float_profile_val= function(valname, mins, scale, maxs, val_min, val_max, val_reset)
		local ret= {
			name= valname, translatable= true,
			menu= nesty_option_menus.adjustable_float, args= {
				name= valname, min_scale= mins, scale= scale, max_scale= maxs,
				reset_value= val_reset, initial_value= function(pn)
					local profile= PROFILEMAN:GetProfile(pn)
					return profile["Get"..valname](profile)
				end,
				set= function(pn, value)
					local profile= PROFILEMAN:GetProfile(pn)
					profile["Set"..valname](profile, value)
				end,
		}}
		ret.value= float_val_func(mins, ret.args.initial_value)
		return setmetatable(ret, mergable_table_mt)
	end,
	bool_profile_val= function(valname)
		local ret= {
			type= "bool",
			name= valname, translatable= true, execute= function(pn)
				local profile= PROFILEMAN:GetProfile(pn)
				profile["Set"..valname](profile, not profile["Get"..valname](profile))
			end,
			value= function(pn)
				local profile= PROFILEMAN:GetProfile(pn)
				return profile["Get"..valname](profile)
			end,
		}
		return setmetatable(ret, mergable_table_mt)
	end,
	float_config_val_args= function(
			conf, field_name, mins, scale, maxs, val_min, val_max)
		local ret= {
			name= field_name, min_scale= mins, scale= scale, max_scale= maxs,
			val_min= val_min, val_max= val_max,
			reset_value= get_element_by_path(conf:get_default(), field_name),
			initial_value= function(pn)
				return get_element_by_path(conf:get_data(pn), field_name) or 0
			end,
			set= function(pn, value)
				set_element_by_path(conf:get_data(pn), field_name, value)
				conf:set_dirty(pn)
				MESSAGEMAN:Broadcast("ConfigValueChanged", {
					config_name= conf.name, field_name= field_name, value= value, pn= pn})
			end,
		}
		return setmetatable(ret, mergable_table_mt)
	end,
	float_config_val= function(
			conf, field_name, mins, scale, maxs, val_min, val_max)
		local ret= {
			name= field_name, translatable= true,
			menu= nesty_option_menus.adjustable_float,
			args= nesty_options.float_config_val_args(conf, field_name, mins, scale, maxs, val_min, val_max),
		}
		ret.value= float_val_func(mins, ret.args.initial_value)
		return setmetatable(ret, mergable_table_mt)
	end,
	float_config_toggle_val= function(conf, field_name, on_val, off_val)
		local ret= {
			type= "bool",
			name= field_name, translatable= true, execute= function(pn)
				local old_val= get_element_by_path(conf:get_data(pn), field_name)
				local new_val= float_toggle_val_toggle_logic(old_val, on_val, off_val)
				conf:set_dirty(pn)
				set_element_by_path(conf:get_data(pn), field_name, new_val)
				MESSAGEMAN:Broadcast("ConfigValueChanged", {
					config_name= conf.name, field_name= field_name, value= new_val, pn= pn})
			end,
			value= function(pn)
				return float_toggle_val_bool_logic(get_element_by_path(conf:get_data(pn), field_name), on_val, off_val)
			end,
		}
		return setmetatable(ret, mergable_table_mt)
	end,
	bool_config_val= function(conf, field_name)
		local ret= {
			type= "bool",
			name= field_name, translatable= true, execute= function(pn)
				local old_val= get_element_by_path(conf:get_data(pn), field_name)
				conf:set_dirty(pn)
				set_element_by_path(conf:get_data(pn), field_name, not old_val)
				MESSAGEMAN:Broadcast("ConfigValueChanged", {
					config_name= conf.name, field_name= field_name, value= new_val, pn= pn})
			end,
			value= function(pn)
				return get_element_by_path(conf:get_data(pn), field_name)
			end,
		}
		return setmetatable(ret, mergable_table_mt)
	end,
	choices_config_val= function(conf, field_name, choices)
		local ret= {
			name= field_name, translatable= true, menu=
				nesty_option_menus.enum_option, args= {
				name= field_name, enum= choices, fake_enum= true,
				obj_get= function(pn) return conf:get_data(pn) end,
				get= function(pn, obj) return get_element_by_path(obj, field_name) end,
				set= function(pn, obj, value)
					set_element_by_path(obj, field_name, value)
					MESSAGEMAN:Broadcast("ConfigValueChanged", {
						config_name= conf.name, field_name= field_name, value= value, pn= pn})
				end,
			},
			value= function(pn)
				return get_element_by_path(conf:get_data(pn), field_name)
			end,
		}
		return setmetatable(ret, mergable_table_mt)
	end,
}
