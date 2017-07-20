-- This file needs to be added to ScreenEdit overlay.lua like this:
--   LoadActor(THEME:GetPathG("", "editor_mods_preview.lua")),

local song_dir= GAMESTATE:GetCurrentSong():GetSongDir()
local mods_filename= "notefield_mods.lua"
local mods_path= song_dir .. mods_filename

local clickable_path= THEME:GetPathG("", "mod_preview_config_value_handler")

local container= false
local button_container= false
local editor_notefield= false
local notefields= {}
local preview_sprite= false
local clickables= {}

local debug_main= Def.ActorFrame{
	InitCommand= function(self) self:visible(false) end
}

local debug_frames= {}
local function make_debug_frame()
	debug_main[#debug_main+1]= Def.ActorFrame{
		InitCommand= function(self)
			debug_frames[#debug_frames+1]= self
		end,
		SetCommand= function(self, params)
			local w= params[2] - params[1]
			local h= params[4] - params[3]
			local x= (params[2] + params[1]) / 2
			local y= (params[4] + params[3]) / 2
			self:xy(x, y)
			self:GetChild("l"):x(w * -.5):setsize(1, h)
			self:GetChild("r"):x(w * .5):setsize(1, h)
			self:GetChild("t"):y(h * -.5):setsize(w, 1)
			self:GetChild("b"):y(h * .5):setsize(w, 1)
		end,
		Def.Quad{
			Name= 'l', InitCommand= function(self)
				self:diffuse{.5, 0, 0, 1}
			end
		},
		Def.Quad{
			Name= 'r', InitCommand= function(self)
				self:diffuse{.5, 0, .5, 1}
			end
		},
		Def.Quad{
			Name= 't', InitCommand= function(self)
				self:diffuse{0, .5, 0, 1}
			end
		},
		Def.Quad{
			Name= 'b', InitCommand= function(self)
				self:diffuse{0, .5, .5, 1}
			end
		},
	}
end
make_debug_frame()

local button_area_width= 96
local button_area_height= 0
local show_button_area_left= 0
local show_button_area_right= 0
local show_button_area_top= 0
local show_button_area_bottom= 0

local function get_config()
	return editor_config:get_data().preview
end

local function reload_mods()
	if FILEMAN:DoesFileExist(mods_path) then
		local mods, custom_mods= lua.load_config_lua(mods_path)
		if type(mods) ~= "table" then return end
		local num_fields= 1
		if type(mods.fields) == "number" and mods.fields > 0 then
			num_fields= mods.fields
		end
		for id, field in ipairs(notefields) do
			field:clear_timed_mods()
			if id > num_fields then
				field:hibernate(math.huge):visible(false)
			else
				field:hibernate(0):visible(true)
			end
		end
		set_simfile_custom_mods(custom_mods)
		organize_and_apply_notefield_mods(notefields, mods)
	end
end

local curr_offset= 0
local function update(self, delta)
	local mx= INPUTFILTER:GetMouseX()
	local my= INPUTFILTER:GetMouseY()
	if mx >= show_button_area_left and
		mx <= show_button_area_right and
		my >= show_button_area_top and
	my <= show_button_area_bottom then
		button_container:visible(true)
	else
		button_container:visible(false)
	end

	for cid= 1, #clickables do
		local click= clickables[cid]
		local dfid= cid*2
		local pos= {click.increase_button:get_screen_clickable_area()}
		debug_frames[dfid]:playcommand("Set", pos)
		dfid= dfid-1
		pos= {click.decrease_button:get_screen_clickable_area()}
		debug_frames[dfid]:playcommand("Set", pos)
	end
	debug_frames[#debug_frames]:playcommand("Set", {show_button_area_left, show_button_area_right, show_button_area_top, show_button_area_bottom})

	local config= get_config()
	if not config.show_preview then return end
	local editor_time= editor_notefield:get_curr_second()
	local display_time= editor_time
	if not config.paused then
		curr_offset= curr_offset + (delta * config.playback_speed)
		if curr_offset > config.max_offset then
			curr_offset= config.min_offset
		end
		display_time= editor_time + curr_offset
	end
	for id= 1, #notefields do
		local field= notefields[id]
		if field:GetVisible() then
			field:set_curr_second(display_time)
		end
	end
end

local function toggle_hide()
	local config= get_config()
	config.show_preview= not config.show_preview
	if config.show_preview then
		container:hibernate(0)
	else
		container:hibernate(math.huge)
	end
end

local function toggle_paused()
	local config= get_config()
	config.paused= not config.paused
end

local full_size= false
local function update_size()
	local zoom= get_config().zoom
	if full_size then zoom= 1 end
	scale_to_fit(preview_sprite, _screen.w * zoom, _screen.h * zoom)
end

local buttons_frame= Def.ActorFrame{
	InitCommand= function(self)
		button_container= self
	end,
}

local function set_or_assert(tab, cont, field)
	tab[field]= rec_find_child(cont, field)
	assert(tab[field], "No " .. field .. " actor in " .. clickable_path)
end
local function set_button(button)
	local w= button:GetWidth() * .5
	local h= button:GetHeight() * .5
	button:set_clickable_area(-w, w, -h, h)
end

local function set_value_text(text, value, prec)
	if prec >= 1 then
		text:settextf("%."..prec.."f", value)
	else
		text:settextf("%i", value)
	end
end

local function make_buttons(name, inc, prec, update_func)
	local id= #clickables+1
	clickables[id]= {
		name= name, inc= inc, prec= prec, update= update_func,
	}
	buttons_frame[#buttons_frame+1]= Def.ActorFrame{
		InitCommand= function(self)
			self:xy(0, (id-1) * 32)
			button_area_height= self:GetY() + 64
			local entry= clickables[id]
			entry.container= self
			set_or_assert(entry, self, "increase_button")
			set_or_assert(entry, self, "decrease_button")
			set_button(entry.increase_button)
			set_button(entry.decrease_button)
			set_or_assert(entry, self, "value_text")
			set_or_assert(entry, self, "name_text")
			assert(entry.value_text.settext, "value_text actor in " .. clickable_path .. " must be a BitmapText")
			assert(entry.name_text.settext, "name_text actor in " .. clickable_path .. " must be a BitmapText")
			entry.name_text:settext(entry.name)
			set_value_text(entry.value_text, get_config()[name], entry.prec)
		end,
		LoadActor(clickable_path),
	}
	make_debug_frame()
	make_debug_frame()
end

local positions= {
	-- x, y, horizalign, vertalign
	{0, 0, left, top, 24, 16},
	{_screen.w, 0, right, top, _screen.w-96, 16},
	{_screen.w, _screen.h, right, bottom, _screen.w-96, _screen.h-(5 * 32)},
	{0, _screen.h, left, bottom, 24, _screen.h-(5 * 32)},
}

local function update_position()
	local config= get_config()
	if config.corner > #positions then
		config.corner= 1
	end
	if config.corner < 1 then
		config.corner= #positions
	end
	local info= positions[config.corner]
	preview_sprite:xy(info[1], info[2]):horizalign(info[3]):vertalign(info[4])
	local bx= info[1]
	local by= info[2]
	if info[3] == left then
		bx= bx + (button_area_width * .5)
	elseif info[3] == right then
		bx= bx - (button_area_width * .5)
	end
	if info[4] == top then
		by= by + (button_area_height * .5)
	elseif info[4] == bottom then
		by= by - (button_area_height * .5)
	end
	bx= info[5]
	by= info[6]
	button_container:xy(bx, by)
	show_button_area_left= bx - 32
	show_button_area_right= bx + 96
	show_button_area_top= by - 32
	show_button_area_bottom= by + (#clickables * 32)
end

make_buttons("min_offset", .1, 1, nil)
make_buttons("max_offset", .1, 1, nil)
make_buttons("playback_speed", .01, 2, nil)
make_buttons("zoom", .01, 2, update_size)
make_buttons("corner", 1, 0, update_position)

local function find_hit_clickable()
	local x= INPUTFILTER:GetMouseX()
	local y= INPUTFILTER:GetMouseY()
	for cid= 1, #clickables do
		if clickables[cid].increase_button:pos_in_clickable_area(x, y) then
			clickables[cid].increase_button:playcommand("Click")
			return clickables[cid], 1
		end
		if clickables[cid].decrease_button:pos_in_clickable_area(x, y) then
			clickables[cid].decrease_button:playcommand("Click")
			return clickables[cid], -1
		end
	end
end

local function input(event)
	if event.type == "InputEventType_Release" then return end
	local config= get_config()
	local button= ToEnumShortString(event.DeviceInput.button)
	if config.show_preview then
		if event.DeviceInput.is_mouse then
			local clicked, inc= find_hit_clickable()
			if clicked then
				if button == "left mouse button" then
					config[clicked.name]= config[clicked.name] + (inc * clicked.inc)
				elseif button == "right mouse button" then
					config[clicked.name]= config[clicked.name] + (inc*10*clicked.inc)
				elseif button == "mousewheel up" then
					config[clicked.name]= config[clicked.name] + clicked.inc
				elseif button == "mousewheel down" then
					config[clicked.name]= config[clicked.name] - clicked.inc
				elseif button == "middle mouse button" then
					config[clicked.name]= editor_config:get_default().preview[clicked.name]
				end
				set_value_text(clicked.value_text, config[clicked.name], clicked.prec)
				if clicked.update then
					clicked.update()
				end
			end
		else
			if button == config.mods_reload_key then
				reload_mods()
			elseif button == config.hide_key then
				toggle_hide()
			elseif button == config.pause_key then
				toggle_paused()
			elseif button == config.full_size_key then
				full_size= not full_size
				update_size()
			elseif button == config.move_key then
				config.corner= config.corner + 1
				update_position()
			elseif button == "KP 7" then
				config.corner= config.corner - 1
				update_position()
			elseif button == config.smaller_key then
				config.zoom= config.zoom - .01
				update_size()
			elseif button == config.larger_key then
				config.zoom= config.zoom + .01
				update_size()
			end
		end
	else
		if button == config.hide_key then
			toggle_hide()
		end
	end
end

local function make_field()
	return Def.NoteField{
		InitCommand= function(self)
			notefields[#notefields+1]= self
			self:hibernate(math.huge)
				:set_vanish_type("FieldVanishType_RelativeToSelf")
				:set_skin("default", {})
				:set_base_values{
					transform_pos_x= _screen.cx, 
					transform_pos_y= _screen.cy,
												}
		end,
	}
end

local frame= Def.ActorFrame{
	OnCommand= function(self)
		container= self
		local screen= SCREENMAN:GetTopScreen()
		screen:AddInputCallback(input)
		editor_notefield= screen:GetChild("NoteFieldEdit")
		for id, field in ipairs(notefields) do
			editor_notefield:share_steps(field)
			field:set_speed_mod(false, 1)
		end
		self:SetUpdateFunction(update)
		if not get_config().show_preview then
			self:hibernate(math.huge)
		end
		update_size()
		update_position()
	end,
	Def.ActorFrameTexture{
		InitCommand= function(self)
			self:setsize(_screen.w, _screen.h)
				:SetTextureName("mod_preview_overlay")
				:EnablePreserveTexture(false)
				:Create()
		end,
		make_field(),
		make_field(),
	},
	Def.Sprite{
		Texture= "mod_preview_overlay", InitCommand= function(self)
			preview_sprite= self
		end,
	},
	buttons_frame,
	debug_main,
}

return frame
