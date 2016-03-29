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
		if not params[key] then params[key]= value end
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
			--Trace(depth .. "child " .. i .. " at " .. cx .. ", " .. cy)
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
			self.container:linear(.1):xy(self.x, self.y)
			if new_size then
				for i, part in ipairs{self.left, self.middle, self.right} do
					part:linear(.1):zoomtoheight(self.h)
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
			self.container:hibernate(math.huge)
		end,
		unhide= function(self)
			self.hidden= false
			self.container:hibernate(0)
		end,
}}

option_item_underlinable_mt= {
	__index= {
		create_actors= function(self, name)
			self.name= name
			self.zoom= 1
			self.width= SCREEN_WIDTH
			self.prev_index= 1
			self.translation_section= "OptionNames"
			self.underline= setmetatable({}, nesty_cursor_mt)
			return Def.ActorFrame{
				Name= name, InitCommand= function(subself)
					self.container= subself
					self:lose_focus()
				end,
				self.underline:create_actors{
					name= "underline", parts_name= "OptionsUnderline"},
				Def.BitmapText{
					Font= "Common Normal", InitCommand= function(subself)
						self.text= subself
						subself:zoom(self.zoom)
						if self.text_style_init then
							self.text_style_init(subself)
						end
					end,
				},
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
		set_underline_color= function(self, color)
			self.underline:diffuse(color)
		end,
		set_text_colors= function(self, main, stroke)
			self.text:diffuse(main):strokecolor(stroke)
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
				self:set_underline(info.underline)
			else
				self.text:settext("")
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
			self.text:settext(get_string_if_translatable(
				self.info.translatable, self.translation_section, t))
			width_limit_text(self.text, self.width, self.zoom)
			self.underline:refit(nil, nil, self.text:GetZoomedWidth(), nil)
		end,
		get_cursor_fit= function(self)
			local ret= {0, 0, 0, self.height + 4}
			if self.text:GetText() ~= "" then
				ret[3]= self.text:GetWidth() + 4
			end
			return ret
		end,
		gain_focus= play_gain_focus,
		lose_focus= play_lose_focus,
}}

option_item_value_mt= {
	__index= {
		create_actors= function(self, name, height)
			self.name= name
			self.zoom= 1
			self.width= SCREEN_WIDTH
			self.prev_index= 1
			self.translation_section= "OptionNames"
			return Def.ActorFrame{
				Name= name, InitCommand= function(subself)
					self.container= subself
					self.text= subself:GetChild("text")
					self.value= subself:GetChild("value")
					self:lose_focus()
				end,
				Def.Quad{
					Name= "example", InitCommand= function(subself)
						self.value_example= subself
						subself:visible(false):horizalign(left)
					end
				},
				normal_text("text", "", nil, nil, nil, nil, nil, left),
				normal_text("value", "", nil, nil, nil, nil, nil, right),
			}
		end,
		set_geo= function(self, width, height, zoom)
			self.width= width
			self.height= height
			self.zoom= zoom
			self.text:zoom(zoom):x(-width/2)
			self.value:zoom(zoom):x(width/2)
			self.value_example:x(width/2 + 4):setsize(height * 2, height)
		end,
		set_text_colors= function(self, main, stroke)
			self.text:diffuse(main):strokecolor(stroke)
			self.value:diffuse(main):strokecolor(stroke)
		end,
		transform= option_item_underlinable_mt.__index.transform,
		set= function(self, info)
			self.info= info
			if info then
				self.text:zoom(self.zoom)
					:settext(get_string_if_translatable(
						info.translatable, self.translation_section, info.text))
				self.value:zoom(self.zoom)
					:settext(get_string_if_translatable(
						info.translatable_value, self.translation_section, info.value))
				local ex_color= is_color_string(info.value)
				if ex_color then
					self.value_example:diffuse(ex_color):visible(true)
				else
					self.value_example:visible(false)
				end
				local twidth= self.text:GetZoomedWidth()
				local vwidth= self.value:GetZoomedWidth()
				if twidth + vwidth + 16 > self.width then
					if vwidth > 0 then
						-- w1 * z1 + w2 * z2 + 16 = w3
						-- z1 = z2
						-- z1 * (w1 + w2) + 16 = w3
						-- z1 * (w1 + w2) = w3 - 16
						-- z1 = (w3 - 16) / (w1 + w2)
						local z= (self.width - 16) / (twidth + vwidth)
						self.text:zoomx(z)
						self.value:zoomx(z)
					else
						width_limit_text(self.text, self.width, self.zoom)
					end
				end
			else
				self.text:settext("")
				self.value:settext("")
				self.value_example:visible(false)
			end
		end,
		get_cursor_fit= function(self)
			local ret= {0, 0, 0, self.height + 4}
			if self.text:GetText() ~= "" then
				ret[1]= self.text:GetX()
				ret[3]= self.text:GetWidth()
			end
			if self.value:GetText() ~= "" then
				if ret[3] > 0 then
					ret[3]= self.value:GetX() - self.text:GetX()
				else
					ret[1]= -self.value:GetWidth()
					ret[3]= self.value:GetX()
				end
			end
			if ret[1] ~= 0 then ret[1]= ret[1] - 2 end
			ret[3]= ret[3] + 4
			return ret
		end,
		set_underline_color= noop_nil,
		set_underline= noop_nil,
		gain_focus= play_gain_focus,
		lose_focus= play_lose_focus,
}}

local option_display_default_params= {
	name= "", x= 0, y= 0, height= 0, el_width= 0, el_height= 0, el_zoom= 0,
	no_heading= false, no_display= false,
	item_mt= option_item_underlinable_mt,
}
nesty_option_display_mt= {
	__index= {
		create_actors= function(self, params)
			add_defaults_to_params(params, option_display_default_params)
			local el_count= 1
			if not params.no_heading then
				params.height= params.height - params.el_height
			end
			if not params.no_display then
				params.height= params.height - params.el_height
			end
			if not no_heading or not no_display then
				params.height= params.height - (params.el_height * .5)
			end
			el_count= math.floor(params.height / params.el_height)
			self.name= params.name
			self.el_width= params.el_width or SCREEN_WIDTH
			self.el_height= params.el_height or line_height
			self.el_zoom= params.el_zoom or 1
			self.no_heading= params.no_heading
			self.no_display= params.no_display
			self.translation_section= "OptionNames"
			local frame= {
				Name= name, InitCommand= function(subself)
					subself:xy(params.x, params.y)
					self.container= subself
					self:regeo_items()
				end,
			}
			local next_y= 0
			if not no_heading then
				local head_y= next_y
				frame[#frame+1]= Def.BitmapText{
					Font= "Common Normal", InitCommand= function(subself)
						self.heading= subself
						subself:xy(0, head_y):zoom(self.el_zoom)
					end
				}
				next_y= next_y + self.el_height
			end
			if not no_display then
				local disp_y= next_y
				frame[#frame+1]= Def.BitmapText{
					Font= "Common Normal", InitCommand= function(subself)
						self.display= subself
						subself:xy(0, disp_y):zoom(self.el_zoom)
					end
				}
				next_y= next_y + self.el_height
			end
			if (not no_heading) or (not no_display) then
				next_y= next_y + self.el_height * .5
			end
			self.scroller= setmetatable({disable_wrapping= true}, item_scroller_mt)
			self.item_mt= params.item_mt
			frame[#frame+1]= self.scroller:create_actors(
				"wheel", el_count, params.item_mt, 0, next_y)
			return Def.ActorFrame(frame)
		end,
		set_underline_color= function(self, color)
			for i, item in ipairs(self.scroller.items) do
				item:set_underline_color(color)
			end
		end,
		set_text_colors= function(self, main, stroke)
			local function set_one(one)
				one:diffuse(main):strokecolor(stroke)
			end
			if not self.no_heading then
				set_one(self.heading)
			end
			if not self.no_display then
				set_one(self.display)
			end
			for i, item in ipairs(self.scroller.items) do
				item:set_text_colors(main, stroke)
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
			if not self.no_heading then
				self.heading:settext(get_string_if_translatable(
					true, self.translation_section, h))
				width_limit_text(self.heading, self.el_width, self.el_zoom)
			end
		end,
		set_display= function(self, d)
			if not self.no_display then
				self.display:settext(
					get_string_if_translatable(
						true, self.translation_section, d))
				width_limit_text(self.display, self.el_width, self.el_zoom)
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

function up_element()
	return {text= "&leftarrow;"}
end

option_set_general_mt= {
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
		update_el_underline= function(self, pos, underline)
			self.info_set[pos].underline= underline
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
					return true
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
					return true
				end,
				Start= function(self)
					if self.info_set[self.cursor_pos].text == up_element().text then
						-- This position is the "up" element that moves the
						-- cursor back up the options tree.
						return false
					end
					if self.interpret_start then
						local menu_ret= {self:interpret_start()}
						if self.scroll_to_move_on_start then
							local pos_diff= 1 - self.cursor_pos
							self.cursor_pos= 1
							self.display:scroll(self.cursor_pos)
						end
						return unpack(menu_ret)
					else
						return false
					end
				end,
				Select= function(self)
					self.cursor_pos= 1
					self.display:scroll(self.cursor_pos)
					return true
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
--     meta= {} -- metatable for the submenu
--     args= {} -- extra args for the initialize function of the metatable
nesty_option_menus.menu= {
	__index= {
		initialize= function(self, pn, initializer_args, no_up, up_text)
			self.init_args= initializer_args
			self.pn= pn
			self.no_up= no_up
			self.up_text= up_text
			self:recall_init()
		end,
		recall_init= function(self)
			self.menu_data= self.init_args
			if type(self.init_args) == "function" then
				self.menu_data= self.init_args(self.pn)
			end
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
			if not self.no_up then
				if self.up_text then
					self.info_set[#self.info_set+1]= {
						text= self.up_text, translatable= true}
				else
					self.info_set[#self.info_set+1]= up_element()
				end
			end
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
		id_plus_up= function(self, id)
			if self.no_up then return id end
			return id + 1
		end,
		id_minus_up= function(self, id)
			if self.no_up then return id end
			return id - 1
		end,
		update_info= function(self, new_menu_data)
			local next_shown= 1
			for i, data in ipairs(new_menu_data) do
				local show= true
				if data.req_func then
					show= show and data.req_func(self.pn)
				end
				if show then
					local disp_slot= self:id_plus_up(next_shown)
					self.shown_data[next_shown]= data
					local disp_text= data.text or data.name
					local underline= data.underline
					if type(underline) == "function" then
						underline= underline(self.pn)
					end
					if not self.info_set[disp_slot] then
						self.info_set[disp_slot]= {}
					end
					self.info_set[disp_slot].text= disp_text
					self.info_set[disp_slot].underline= underline
					self.info_set[disp_slot].value= data.value
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
				self.info_set[self:id_plus_up(index)]= nil
				if self.display then
					self.display:set_element_info(self:id_plus_up(index), nil)
				end
			end
			self.menu_data= new_menu_data
		end,
		recheck_levels= function(self)
			self:reset_info()
		end,
		set_status= function(self)
			if self.display then
				self.display:set_heading(self.name or "")
				self.display:set_display(self.menu_data.status or "")
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
			local data= self.shown_data[self:id_minus_up(self.cursor_pos)]
			if self.special_handler then
				local handler_ret= self.special_handler(self, data)
				if handler_ret.recall_init then
					self:recall_init()
					return true
				elseif handler_ret.ret_data then
					return unpack(handler_ret.ret_data)
				else
					return false
				end
			else
				if data then
					return true, data
				else
					return false
				end
			end
		end,
		get_item= function(self, pos)
			pos= self:id_minus_up(pos or self.cursor_pos)
			if pos == 0 then
				return self.info_set[1]
			end
			return self.shown_data[pos]
		end,
		get_item_name= function(self, pos)
			pos= self:id_minus_up(pos or self.cursor_pos)
			local shown= self.shown_data[pos]
			if shown then
				return shown.name or shown.text
			end
			return self.up_text or ""
		end
}}

nesty_option_menus.boolean_option= {
	__index= {
		initialize= function(self, pn, extra)
			self.name= extra.name
			self.pn= pn
			self.cursor_pos= 1
			self.get= extra.get
			self.set= extra.set
			local curr= extra.get(pn)
			self.info_set= {
				up_element(),
				{text= extra.true_text, translatable= true, underline= curr},
				{text= extra.false_text, translatable= true, underline= not curr}}
		end,
		set_status= function(self)
			self.display:set_heading(self.name)
			self.display:set_display("")
		end,
		interpret_start= function(self)
			if self.cursor_pos == 1 then return false end
			local curr= self.cursor_pos == 2
			self.set(self.pn, curr)
			self:update_el_underline(2, curr)
			self:update_el_underline(3, not curr)
			return true
		end
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
			self.set= extra.set
			check_member("set")
			self.val_min= extra.val_min
			self.val_max= extra.val_max
			self.val_to_text= extra.val_to_text or to_text_default
			self.scale_to_text= extra.scale_to_text or to_text_default
			--local scale_text= THEME:GetString("OptionNames", "scale")
			--self.pi_text= THEME:GetString("OptionNames", "pi")
			local scale_text= "scale"
			self.pi_text= "pi"
			self.info_set= {
				up_element(),
				{text= "+"..self.scale_to_text(self.pn, 10^self.scale)},
				{text= "-"..self.scale_to_text(self.pn, 10^self.scale)},
				{text= scale_text.."*10"}, {text= scale_text.."/10"},
				{text= "Round", translatable= true},
				{text= "Reset", translatable= true}}
			self.menu_functions= {
				function() return false end, -- up element
				function() -- increment
					self:set_new_val(self.current_value + 10^self.scale)
					return true
				end,
				function() -- decrement
					self:set_new_val(self.current_value - 10^self.scale)
					return true
				end,
				function() -- scale up
					self:set_new_scale(self.scale + 1)
					return true
				end,
				function() -- scale down
					self:set_new_scale(self.scale - 1)
					return true
				end,
				function() -- round
					self:set_new_val(math.round(self.current_value))
					return true
				end,
				function() -- reset
					local new_scale, new_value=
						find_scale_for_number(self.reset_value, self.min_scale)
					self:set_new_scale(new_scale)
					self:set_new_val(new_value)
					return true
				end,
			}
			if extra.is_angle then
				-- insert the pi option before the Round option.
				local pi_pos= #self.info_set-1
				local function pi_function()
					self.pi_exp= not self.pi_exp
					if self.pi_exp then
						self:update_el_text(6, "/"..self.pi_text)
					else
						self:update_el_text(6, "*"..self.pi_text)
					end
					self:set_new_val(self.current_value)
					return true
				end
				table.insert(self.info_set, pi_pos, {text= "*"..self.pi_text})
				table.insert(self.menu_functions, pi_pos, pi_function)
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
				if self.pi_exp then
					val_text= val_text .. "*" .. self.pi_text
				end
				self.display:set_display(val_text)
			end
		end,
		cooked_val= function(self, nval)
			if self.pi_exp then return nval * math.pi end
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
	__index= {
		initialize= function(self, pn, extra)
			self.name= extra.name
			self.pn= pn
			self.enum_vals= {}
			self.info_set= {up_element()}
			self.cursor_pos= 1
			self.get= extra.get
			self.set= extra.set
			self.fake_enum= extra.fake_enum
			self.ops_obj= extra.obj_get(pn)
			local cv= self:get_val()
			for i, v in ipairs(extra.enum) do
				self.enum_vals[#self.enum_vals+1]= v
				self.info_set[#self.info_set+1]= {
					text= self:short_string(v), translatable= true, underline= v == cv}
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
					if info.underline then
						self:update_el_underline(i, false)
					end
				end
				self:update_el_underline(self.cursor_pos, true)
				if self.ops_obj then
					self.set(self.pn, self.ops_obj, self.enum_vals[self.cursor_pos-1])
				else
					self.set(self.pn, self.enum_vals[self.cursor_pos-1])
				end
				self.display:set_display(self:short_string(self:get_val()))
				return true
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
			self.display:set_display(self:short_string(self:get_val()))
		end
}}

function set_nesty_option_metatables()
	for k, set in pairs(nesty_option_menus) do
		setmetatable(set.__index, option_set_general_mt)
	end
end
set_nesty_option_metatables()

-- This exists to hand to menus that pass out of view but still exist.
local fake_display= {is_fake= true}
for k, v in pairs(nesty_option_display_mt.__index) do
	fake_display[k]= function() end
end

local menu_stack_param_defaults= {
	name= "", x= 0, y= 0, width= _screen.w, height= _screen.h, num_displays= 1,
	el_height= line_height, zoom= 1, no_heading= false, no_display= false,
	item_mt= option_item_underlinable_mt,
	display_mt= nesty_option_display_mt,
	cursor_above_options= true,
	cursor_mt= nesty_cursor_mt,
}
nesty_menu_stack_mt= {
	__index= {
		create_actors= function(self, params)
			add_defaults_to_params(params, menu_stack_param_defaults)
			self.name= params.name
			self.pn= params.pn
			self.options_set_stack= {}
			self.zoom= params.zoom
			self.el_height= params.el_height or line_height
			local pcolor= PlayerColor(params.pn)
			local frame= {
				Name= name, InitCommand= function(subself)
					subself:xy(params.x, params.y)
					self.container= subself
					self.cursor:refit(nil, nil, 20, self.el_height)
					for i, disp in ipairs(self.displays) do
						disp:set_underline_color(pcolor)
					end
				end
			}
			self.displays= {}
			for i= 1, params.num_displays do
				self.displays[#self.displays+1]= setmetatable({}, params.display_mt)
			end
			local sep= params.width / #self.displays
			if #self.displays == 1 then sep= 0 end
			local off= sep / 2
			self.cursor= setmetatable({}, params.cursor_mt)
			if not cursor_above_options then
				frame[#frame+1]= self.cursor:create_actors{
					name= "cursor", pn= params.pn}
			end
			local disp_el_width_limit= (params.width / #self.displays) - 8
			for i, disp in ipairs(self.displays) do
				frame[#frame+1]= disp:create_actors{
					name= "disp" .. i, x= off+sep * (i-1), y= 0, height= params.height,
					el_width= disp_el_width_limit, el_height= self.el_height,
					el_zoom= self.zoom, no_heading= params.no_heading,
					no_display= params.no_display, item_mt= params.item_mt}
			end
			if cursor_above_options then
				frame[#frame+1]= self.cursor:create_actors{
					name= "cursor", pn= params.pn}
			end
			return Def.ActorFrame(frame)
		end,
		set_translation_section= function(self, section)
			for i, disp in ipairs(self.displays) do
				disp:set_translation_section(section)
			end
		end,
		assign_displays= function(self, start)
			local oss= self.options_set_stack
			for i= #oss, 1, -1 do
				oss[i]:set_display(self.displays[start] or fake_display)
				start= start - 1
			end
		end,
		lose_focus_top_display= function(self)
			local top_display= math.min(#self.displays, #self.options_set_stack)
			if self.displays[top_display] then
				self.displays[top_display]:lose_focus_items()
			end
		end,
		push_display_stack= function(self)
			local use_display= math.min(#self.displays, #self.options_set_stack+1)
			self:assign_displays(use_display - 1)
			self:hide_unused_displays(use_display)
			return use_display
		end,
		pop_display_stack= function(self)
			local oss= self.options_set_stack
			local use_display= math.min(#self.displays, #oss)
			self:assign_displays(use_display)
			for i= #oss, 1, -1 do
				local curr_set= oss[i]
				if not curr_set.display.is_fake then
					if curr_set.recall_init_on_pop then
						curr_set:recall_init()
					end
					curr_set:recheck_levels()
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
		push_options_set_stack= function(
				self, new_set_meta, new_set_initializer_args, base_exit, no_up)
			self:lose_focus_top_display()
			local oss= self.options_set_stack
			local use_display= self:push_display_stack()
			local nos= setmetatable({}, new_set_meta)
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
		pop_options_set_stack= function(self)
			self:lose_focus_top_display()
			local oss= self.options_set_stack
			if #oss > 0 then
				local former_top= oss[#oss]
				if former_top.destructor then former_top:destructor(self.pn) end
				oss[#oss]= nil
				self:pop_display_stack()
			end
			self:update_cursor_pos()
		end,
		clear_options_set_stack= function(self)
			while #self.options_set_stack > 0 do
				self:pop_options_set_stack()
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
			local oss= self.options_set_stack
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
			local oss= self.options_set_stack
			local top_set= oss[#oss]
			local handled, new_set_data= top_set:interpret_code(code)
			if handled then
				if new_set_data then
					if new_set_data.meta == "external_interface" then
						self:enter_external_mode()
						new_set_data.extern(self, new_set_data.args, self.pn)
						self.deextern= new_set_data.deextern
					elseif new_set_data.meta == "execute" then
						new_set_data.execute(self.pn)
						top_set:recheck_levels()
					else
						local nargs= new_set_data.args
						if new_set_data.exec_args and type(nargs) == "function" then
							nargs= nargs(self.pn)
						end
						self:push_options_set_stack(new_set_data.meta, nargs)
					end
				end
			else
				if (code == "Start" or code == "Back") and #oss > 1 then
					handled= true
					self:pop_options_set_stack()
				end
			end
			self:update_cursor_pos()
			return handled
		end,
		update_cursor_pos= function(self)
			local tos= self.options_set_stack[#self.options_set_stack]
			if not tos then return end
			local item= tos:get_cursor_element()
			if item then
				item:gain_focus()
				local xmn, xmx, ymn, ymx= rec_calc_actor_extent(item.container)
				local xp, yp= rec_calc_actor_pos(item.container)
				local xs, ys= rec_calc_actor_pos(self.container)
				xp= xp - xs
				yp= yp - ys
				self.cursor:refit(xp, yp, xmx - xmn + 4, ymx - ymn + 4)
			end
		end,
		refit_cursor= function(self, fit)
			self.cursor:refit(fit[1], fit[2], fit[3], fit[4])
		end,
		can_exit_screen= function(self)
			local oss= self.options_set_stack
			local top_set= oss[#oss]
			return #oss <= 1 and (not top_set or top_set:can_exit())
		end,
		top_menu= function(self)
			return self.options_set_stack[#self.options_set_stack]
		end,
		get_cursor_item= function(self)
			local top_set= self.options_set_stack[#self.options_set_stack]
			if top_set.get_item then
				return top_set:get_item()
			end
			return nil
		end,
		get_cursor_item_name= function(self)
			local top_set= self.options_set_stack[#self.options_set_stack]
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
	if not per_player_menus[pn]:interpret_code(button) then
		if button == "Start" then
			if close_menu_callback then close_menu_callback(pn) end
			return true
		end
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

local function float_toggle_val_underline_logic(old_val, on_val, off_val)
	local mid_val= (on_val + off_val) * .5
	if on_val > off_val then
		return old_val > mid_val
	else
		return old_val < mid_val
	end
end

nesty_options= {
	float_pref_val= function(valname, min_scale, scale, max_scale, val_min, val_max, val_reset)
		return {
			name= valname, translatable= true,
			meta= nesty_option_menus.adjustable_float,
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
	end,
	float_song_mod_val= function(valname, min_scale, scale, max_scale, val_min, val_max, val_reset)
		return {
			name= valname, translatable= true,
			meta= nesty_option_menus.adjustable_float,
			args= {
				name= valname, min_scale= min_scale, scale= scale,
				max_scale= max_scale, val_min= val_min, val_max= val_max,
				reset_value= val_reset,
				initial_value= function()
					local song_ops= GAMESTATE:GetSongOptionsObject("ModsLevel_Preferred")
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
	end,
	float_song_mod_toggle_val= function(valname, on_val, off_val)
		return {
			name= valname, meta= "execute", translatable= true,
			execute= function()
				local song_ops= GAMESTATE:GetSongOptionsObject("ModsLevel_Preferred")
				local old_val= song_ops[valname](song_ops)
				local new_val= float_toggle_val_toggle_logic(old_val, on_val, off_val)
				song_ops[valname](song_ops, new_val)
				-- Apply the change to the current and song levels too, so that if
				-- this occurs during gameplay, it takes effect immediately.
				GAMESTATE:ApplyPreferredSongOptionsToOtherLevels()
			end,
			underline= function()
				local song_ops= GAMESTATE:GetSongOptionsObject("ModsLevel_Preferred")
				local old_val= song_ops[valname](song_ops)
				return float_toggle_val_underline_logic(old_val, on_val, off_val)
			end,
		}
	end,
	bool_player_mod_val= function(valname)
		return {
			name= valname, meta= "execute", translatable= true,
			execute= function(pn)
				local plops= GAMESTATE:GetPlayerState(pn):get_player_options_no_defect("ModsLevel_Preferred")
				local new_val= not plops[valname](plops)
				plops[valname](plops, new_val)
			end,
			underline= function(pn)
				local plops= GAMESTATE:GetPlayerState(pn):get_player_options_no_defect("ModsLevel_Preferred")
				return plops[valname](plops)
			end,
		}
	end,
	float_config_val_args= function(
			conf, field_name, mins, scale, maxs, val_min, val_max)
		return {
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
	end,
	float_config_val= function(
			conf, field_name, mins, scale, maxs, val_min, val_max)
		return {
			name= field_name, translatable= true,
			meta= nesty_option_menus.adjustable_float,
			args= nesty_options.float_config_val_args(conf, field_name, mins, scale, maxs, val_min, val_max),
		}
	end,
	float_config_toggle_val= function(conf, field_name, on_val, off_val)
		return {
			name= field_name, meta= "execute", translatable= true,
			execute= function(pn)
				local old_val= get_element_by_path(conf:get_data(pn), field_name)
				local new_val= float_toggle_val_toggle_logic(old_val, on_val, off_val)
				set_element_by_path(conf:get_data(pn), field_name, new_val)
				MESSAGEMAN:Broadcast("ConfigValueChanged", {
					config_name= conf.name, field_name= field_name, value= new_val, pn= pn})
			end,
			underline= function(pn)
				return float_toggle_val_underline_logic(get_element_by_path(conf:get_data(pn), field_name), on_val, off_val)
			end,
		}
	end,
	bool_config_val= function(conf, field_name)
		return {
			name= field_name, meta= "execute", translatable= true,
			execute= function(pn)
				local old_val= get_element_by_path(conf:get_data(pn), field_name)
				set_element_by_path(conf:get_data(pn), field_name, not old_val)
				MESSAGEMAN:Broadcast("ConfigValueChanged", {
					config_name= conf.name, field_name= field_name, value= new_val, pn= pn})
			end,
			underline= function(pn)
				return get_element_by_path(conf:get_data(pn), field_name)
			end,
		}
	end,
}
