local num_displays, menu_width, menu_height, menu_zoom, menu_x, menu_y, item_spacing = ...
num_displays= num_displays or 1
menu_width= menu_width or _screen.w * .45
menu_height= menu_height or _screen.h * .9
menu_zoom= menu_zoom or 1
menu_x= menu_x or _screen.w * .05
menu_y= menu_y or _screen.h * .05
item_spacing = item_spacing or 24

local function button_click(self)
	self:stoptweening()
		:linear(.1):diffusealpha(0.7):zoomx(0.95)
		:linear(.1):zoomx(1):diffusealpha(1)
end

local function show(self) self:visible(true) end
local function hide(self) self:visible(false) end

-- Was going to set these repeatedly for each part of the menu item, then
-- decided to just put them on the ActorFrame of the item as a whole.
local function item_part_on(self)
	self:diffusealpha(0):smooth(0.2):diffusealpha(1)
end
local function item_part_off(self)
	self:smooth(0.2):diffusealpha(0)
end

local smw= menu_width / menu_zoom
local smh= menu_height / menu_zoom
local display_pad= smw * .01
local display_width= smw / num_displays
local item_width= display_width - (display_pad * 2)
local num_items= smh / item_spacing
local adjuster_size= item_spacing * .45
local adjuster_pad= item_spacing * .05
local value_adjuster_pad= 2
local value_width= 64
local value_height= item_spacing * .95
local name_value_pad= 4
local name_width= item_width - adjuster_size - value_adjuster_pad - value_width - name_value_pad
local name_x= item_width * -.5
local value_x= name_width + name_value_pad + value_width
local adjuster_x= value_x + value_adjuster_pad + (adjuster_size * .5) + 8

local function make_adjuster(name, bzy)
	return Def.Sprite{
		Name= name, Texture= THEME:GetPathG("", "up_button.png"),
		InitCommand= function(self)
			scale_to_fit(self, adjuster_size, adjuster_size)
			-- scale_to_fit sets zoom, but zoom is used in the click animation, so
			-- move the zoom to basezoom.
			local zoom= self:GetZoom()
			self:basezoom(zoom):zoom(1)
			local hs= adjuster_size * .5
			local yp= (hs + adjuster_pad) * bzy
			self:xy(adjuster_x, -yp):basezoomy(bzy * zoom)
			click_area.wh(self, hs/zoom, hs/zoom)
		end,
		ClickCommand= button_click,
		ShowCommand= show,
		HideCommand= hide,
	}
end

local value_type_handlers= {
	["nil"]= function(self, value)
		self:GetChild("image"):visible(false)
		return self:GetChild("text"):settext("")
	end,
	ms= function(self, value)
		self:GetChild("image"):visible(false)
		return self:GetChild("text"):settextf("%ims", math.round(value*1000))
	end,
	percent= function(self, value)
		self:GetChild("image"):visible(false)
		return self:GetChild("text"):settextf("%i%%", math.round(value*100))
	end,
	number= function(self, value)
		self:GetChild("image"):visible(false)
		if value - math.floor(value) > 0.01 then
			return self:GetChild("text"):settextf("%.2f", value)
		else
			return self:GetChild("text"):settextf("%i", value)
		end
	end,
	time= function(self, value)
		self:GetChild("image"):visible(false)
		return self:GetChild("text"):settext(secs_to_str(value))
	end,
	string= function(self, value)
		self:GetChild("image"):visible(false)
		return self:GetChild("text"):settext(value)
	end,
	boolean= function(self, value)
		self:GetChild("text"):settext("")
		local image= self:GetChild("image")
		image:visible(true)
		image:Load(THEME:GetPathG("", "menu_icons/bool"))
		scale_to_fit(image, value_width, value_height)
		if value then
			image:setstate(1)
		else
			image:setstate(0)
		end
		return image
	end,
	enum= function(self, value)
		self:GetChild("image"):visible(false)
		return self:GetChild("text"):settext(value)
	end,
	default= function(self, value)
		self:GetChild("image"):visible(false)
		return self:GetChild("text"):settext(tostring(value))
	end,
}

local function make_item()
	return Def.ActorFrame{
		Name= "item",
		InitCommand= function(self)
			-- Clickable area used to give this item focus.
			click_area.owh(self, item_width, adjuster_size)
			self:visible(false)
		end,
		OnCommand= item_part_on,
		OffCommand= item_part_off,
		PlayerizeCommand= function(self, pn)
			
		end,
		SetItemCommand= function(self, info)
			-- When the item should display something.
			self:visible(true)
		end,
		RefreshItemCommand= function(self, info)
			-- The item was displaying something, but is assigned something else.
			self:visible(true)
		end,
		ClearItemCommand= function(self)
			-- When the item should display nothing.
			self:visible(false)
		end,
		SetTypeHintCommand= function(self, type_hint)
			-- type_hint is from the menu data.
			if type_hint.main == "submenu" then
				local image= self:GetChild("value"):GetChild("image")
				image:Load(THEME:GetPathG("", "menu_icons/menu.png")):visible(true)
				scale_to_fit(image, value_width, value_height)
			end
		end,
		ClickCommand= function(self)
			-- When player pushes start on the item, or clicks the name.
			button_click(self:GetChild("name"))
		end,
		ResetCommand= function(self)
			-- When the player resets to default value instead of using adjust_up
			-- or adjust_down.
			button_click(self:GetChild("value"))
		end,
		ActiveCommand= function(self)
			self:visible(true)
		end,
		InactiveCommand= function(self)
			self:visible(false)
		end,
		ScrollCommand= function(self, params)
			one_dimension_scroll(self, "y", "linear", .1, item_spacing, params.from, params.to, 0, params.num_items-1)
			if params.scroll_type == "normal" then
			elseif params.scroll_type == "off" then
				-- Item will not be visible or interactive afterwards.
				self:queuecommand("ClearItem")
			elseif params.scroll_type == "on" then
				-- Item moves from not visible, to visible.
				self:visible(true)
			elseif params.scroll_type == "open" then
				-- Something added to menu, animate opening from another item
			elseif params.scroll_type == "close" then
				-- Something removed from menu, animate closing to another item
			elseif params.scroll_type == "first" then
				-- First time a submenu is opened.
				self:finishtweening()
			end
		end,
		GainFocusCommand= function(self)
			-- When the cursor moves to this item.
		end,
		LoseFocusCommand= function(self)
			-- When the cursor moves to another item.
		end,
		Def.BitmapText{
			Name= "name", Font= "Common Condensed",
			InitCommand= function(self)
				self:horizalign(left)
					:diffuse(color("#3D1D23"))
			end,
			SetNameCommand= function(self, name)
				self:settext(name)
				-- Clickable area used for clicking on the name to interact with the
				-- item.  Any toggle choice or submenu will use this.
				-- The clickable area needs to be reset when the text changes because
				-- the text alignment moves it.
				-- The clickable area is affected by zoom.
				local zoom= self:GetZoom()
				click_area.lwh(self, -self:GetWidth() / 2, name_width/zoom, item_spacing * .5)
			end,
			ClickCommand= button_click,
		},
		Def.ActorFrame{
			Name= "value", InitCommand= function(self)
				self:x(value_x)
			end,
			SetValueCommand= function(self, info)
				if not info then
					value_type_handlers["nil"](self)
					return
				end
				local vtype= info[1]
				local value= info[2]
				local handler= value_type_handlers[vtype] or value_type_handlers.default
				local visible_part= handler(self, value)
				local vis_width= visible_part:GetZoomedWidth()
				click_area.lwh(self, -vis_width, vis_width, item_spacing * .5)
			end,
			ClickCommand= button_click,
			Def.BitmapText{
				Name= "text", Font= "Common Condensed",
				InitCommand= function(self)
					self:horizalign(right)
						:diffuse(color("#AC214A"))
				end,
			},
			Def.Sprite{
				Name= "image", InitCommand= function(self)
					self:horizalign(right):animate(false)
				end,
			}
		},
		make_adjuster("adjust_up", 1),
		make_adjuster("adjust_down", -1),
	}
end

local function make_display(num_items)
	local frame= Def.ActorFrame{
		Name= "display",
		InitCommand= function(self)
			-- Clickable area used to detect when the mouse enters this display.
			click_area.ltwh(self, -display_pad, item_spacing * -.5, display_width, num_items * item_spacing)
		end,
		-- Funny story: Actor won't work because it would never be rendered.
		-- A transparent Quad won't work because fully transparent actors
		-- aren't rendered.
		-- But an ActorFrame with nothing in it is rendered, which converts the
		-- relative clickable area to usable screen coordinates.
		Def.ActorFrame{
			Name= "mouse_scroll_area", InitCommand= function(self)
				-- Area where the mouse wheel will scroll items instead of changing
				-- values.
				click_area.ltwh(self, 0, item_spacing * -.5, name_width, num_items * item_spacing)
			end,
		},
		PlayerizeCommand= function(self, pn)
			
		end,
		ActiveCommand= function(self)
			self:visible(true)
		end,
		InactiveCommand= function(self)
			self:visible(false)
		end,
		ScrollCommand= function(self, params)
			one_dimension_scroll(self, "x", "bounceend", .2, 15, params.from, params.to, 0, params.num_items-1)
			if params.scroll_type == "normal" then
			elseif params.scroll_type == "off" then
				-- Item will not be visible or interactive afterwards.
				self:queuecommand("Inactive")
			elseif params.scroll_type == "on" then
				-- Item moves from not visible, to visible.
				self:visible(true)
			elseif params.scroll_type == "first" then
				-- First time a submenu is opened.
				self:finishtweening()
			end
		end,
		GainFocusCommand= function(self)
			
		end,
		LoseFocusCommand= function(self)
			
		end,
		OpenSubmenuCommand= function(self, info)
			
		end,
		RefreshSubmenuCommand= function(self, info)
			
		end,
		CloseSubmenuCommand= function(self)
			self:visible(false)
		end,
	}
	-- Half of the items will be used to make scrolling look good.
	for i= 1, num_items*2 do
		frame[#frame+1]= make_item()
	end
	return frame
end

local menu_frame= Def.ActorFrame{
	Name= "menu",
	InitCommand= function(self)
		click_area.owoh(self, display_width * num_displays, num_items * item_spacing)
		self:zoom(menu_zoom):xy(menu_x, menu_y)
	end,
	PlayerizeCommand= function(self, pn)
		self:GetChild("cursor"):diffuse(ColorLightTone(PlayerColor(pn)))
	end,
	OpenMenuCommand= function(self)
		self:visible(true)
	end,
	CloseMenuCommand= function(self)
		self:visible(false)
	end,
	ShowCommand= function(self)
		self:visible(true)
	end,
	HideCommand= function(self)
		self:visible(false)
	end,
	Def.ActorFrame{
		Name= "cursor", InitCommand= function(self)
			local left_part= self:GetChild("left")
			local middle_part= self:GetChild("middle")
			local right_part= self:GetChild("right")
			local w= item_width
			left_part:horizalign(right):x(w*-.5)
			middle_part:zoomtowidth(w)
			right_part:horizalign(left):x(w*.5)
		end,
		MoveCommand= function(self, params)
			self:stoptweening():linear(.1)
				:xy(params.x + (item_width * .5), params.y)
		end,
		-- AdjustMode and NormalMode are for two_direction input mode
		-- (left, right, and start buttons only)
		-- So adjust mode means that left and right will change the value instead
		-- of moving the cursor.
		-- This is of course a placeholder command that Lirodon is supposed to
		-- change.
		NormalModeCommand= function(self)
			self:linear(.1):rotationz(0)
		end,
		AdjustModeCommand= function(self)
			self:linear(.1):rotationz(90)
		end,
		LoadActor(THEME:GetPathG("OptionsCursor", "Middle"))..{Name= "middle"},
		LoadActor(THEME:GetPathG("OptionsCursor", "Left"))..{Name= "left"},
		LoadActor(THEME:GetPathG("OptionsCursor", "Right"))..{Name= "right"},
	},
}
-- Half of the displays will be used to make scrolling look good.
for i= 1, num_displays*2 do
	menu_frame[#menu_frame+1]= make_display(num_items)
end

return menu_frame
