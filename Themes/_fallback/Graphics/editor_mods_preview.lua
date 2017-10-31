-- This file needs to be added to ScreenEdit overlay.lua like this:
--   LoadActor(THEME:GetPathG("", "editor_mods_preview.lua")),
-- Don't try to edit this file to style the preview to your theme.
-- Edit the file pointed to by clickable_path to style the buttons and text
-- for changing the config.

local song_dir= GAMESTATE:GetCurrentSong():GetSongDir()
local mods_filename= "notefield_mods.lua"
local mods_path= song_dir .. mods_filename

local clickable_path= THEME:GetPathG("", "mod_preview_config_handler")

local container= false
local button_container= false
local editor_notefield= false
local play_notefield= false
local edit_screen= false
local notefields= {}
local preview_sprite= false
local clickables= {}

local debug_frames= false

local button_area_width= 96
local button_area_height= 0
local show_button_area_left= 0
local show_button_area_right= 0
local show_button_area_top= 0
local show_button_area_bottom= 0

local curr_edit_state= "Edit"

local function get_config()
	return editor_config:get_data().preview
end

local function update_should_be_hidden()
	if curr_edit_state == "Edit" and get_config().show_preview then
		container:hibernate(0)
	else
		container:hibernate(math.huge)
	end
end

local function get_current_stepstype()
	local steps= GAMESTATE:GetCurrentSteps(PLAYER_1)
	if steps then return steps:GetStepsType() end
	steps= GAMESTATE:GetCurrentSteps(PLAYER_2)
	if steps then return steps:GetStepsType() end
	return "StepsType_Invalid"
end

local pn_to_id= PlayerNumber:Reverse()

local function reload_mods()
	if FILEMAN:DoesFileExist(mods_path) then
		local mods, custom_mods= lua.load_config_lua(mods_path)
		if type(mods) ~= "table" then return end
		local num_fields= 1
		if type(mods.fields) == "number" and mods.fields > 0 then
			num_fields= mods.fields
		end
		for pn, field in pairs(notefields) do
			field:clear_timed_mods()
			field:clear_to_base_skin()
			if pn_to_id[pn] >= num_fields then
				field:hibernate(math.huge):visible(false)
			else
				field:hibernate(0):visible(true)
			end
		end
		set_simfile_custom_mods(custom_mods)
		organize_and_apply_notefield_mods(notefields, mods)
		return true
	else
		local num_fields= 1
		for pn, field in pairs(notefields) do
			field:clear_timed_mods()
			field:clear_to_base_skin()
			if pn_to_id[pn] > num_fields then
				field:hibernate(math.huge):visible(false)
			else
				field:hibernate(0):visible(true)
			end
		end
	end
	return false
end

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

local white= {.75, .5, .25, 1}

local function add_line_to_verts(pa, pb, verts)
	local to= {pb[1] - pa[1], pb[2] - pa[2], 0}
	local left= normalize(cross_product(to, {0, 0, 1}))
	verts[#verts+1]= {{pa[1] + left[1], pa[2] + left[2], 0}, white}
	verts[#verts+1]= {{pb[1] + left[1], pb[2] + left[2], 0}, white}
	verts[#verts+1]= {{pb[1] - left[1], pb[2] - left[2], 0}, white}
	verts[#verts+1]= {{pa[1] - left[1], pa[2] - left[2], 0}, white}
end

local function add_area_to_verts(area, verts)
	for p= 2, #area do
		add_line_to_verts(area[p-1], area[p], verts)
	end
	add_line_to_verts(area[#area], area[1], verts)
end

local function update_debug_frames()
	local verts= {}
	add_area_to_verts(button_container:get_screen_clickable_area(), verts)
	for cid= 1, #clickables do
		local click= clickables[cid]
		if click.increase_button then
			add_area_to_verts(click.increase_button:get_screen_clickable_area(), verts)
			add_area_to_verts(click.decrease_button:get_screen_clickable_area(), verts)
		elseif click.checkbox then
			add_area_to_verts(click.checkbox:get_screen_clickable_area(), verts)
		end
	end
	debug_frames:SetVertices(verts)
end

local curr_offset= 0
local buttons_visible= nil
local function update(self, delta)
	if not button_container then return end
	local mx= INPUTFILTER:GetMouseX()
	local my= INPUTFILTER:GetMouseY()
	if button_container:pos_in_clickable_area(mx, my) then
		if not buttons_visible then
			button_container:playcommand("Show")
			buttons_visible= true
		end
	else
		if buttons_visible then
			button_container:playcommand("Hide")
			buttons_visible= false
		end
	end

	local new_edit_state= ToEnumShortString(edit_screen:GetEditState())
	if new_edit_state ~= curr_edit_state then
		curr_edit_state= new_edit_state
		update_should_be_hidden()
	end
	--update_debug_frames()

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
	for pn, field in pairs(notefields) do
		if field:GetVisible() then
			field:set_curr_second(display_time)
		end
	end
end

local function toggle_hide()
	local config= get_config()
	config.show_preview= not config.show_preview
	update_should_be_hidden()
	return config.show_preview
end

local function toggle_paused()
	local config= get_config()
	config.paused= not config.paused
	return config.paused
end

local full_screen= false
local function update_size()
	local zoom= get_config().zoom
	if full_screen then zoom= 1 end
	scale_to_fit(preview_sprite, _screen.w * zoom, _screen.h * zoom)
end

local function toggle_full_screen()
	full_screen= not full_screen
	update_size()
	return full_screen
end

local positions= {
	-- x, y, horizalign, vertalign
	{0, 0, left, top},
	{_screen.w, 0, right, top},
	{_screen.w, _screen.h, right, bottom},
	{0, _screen.h, left, bottom},
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
	button_container:playcommand("Position", {corner= config.corner})
end

local function set_or_assert(tab, cont, field, name)
	tab[field]= rec_find_child(cont, field)
	assert(tab[field], "No " .. field .. " actor in " .. name)
end
local function set_button(button)
	local w= button:GetWidth() * .5
	local h= button:GetHeight() * .5
	button:set_clickable_area{{-w, -h}, {w, -h}, {w, h}, {-w, h}}
end

local function set_value_text(text, value, prec)
	if prec >= 1 then
		text:settextf("%."..prec.."f", value)
	else
		text:settextf("%i", value)
	end
end

local buttonable_config_info= {
	{"max_offset", .1, 1, nil},
	{"min_offset", .1, 1, nil},
	{"playback_speed", .01, 2, nil},
	{"zoom", .01, 2, update_size},
	{"corner", 1, 0, update_position},
	{"paused", toggle_paused},
	{"show_preview", toggle_hide},
	{"full_screen", toggle_full_screen},
	{"reload_mods", reload_mods},
}

local keymap= {}

local function find_clickable(name)
	for cid= 1, #clickables do
		if clickables[cid].name == name then
			return clickables[cid]
		end
	end
end

local function init_keymap()
	local config= get_config()
	keymap= {
		[config.mods_reload_key]= {
			update= reload_mods, click= find_clickable("reload_mods")},
		[config.hide_key]= {
			update= toggle_hide, click= find_clickable("show_preview")},
		[config.pause_key]= {
			update= toggle_paused, click= find_clickable("paused")},
		[config.full_screen_key]= {
			update= toggle_full_screen, click= find_clickable("full_screen")},
	}
end

local buttons_frame= Def.ActorFrame{
	InitCommand= function(self)
		local n, cont= next(self:GetChildren())
		button_container= cont
		-- Use clickable area of the main container for hiding it when the mouse
		-- moves away.
		local w= button_container:GetWidth()
		local h= button_container:GetHeight()
		button_container:set_clickable_area{{0, 0}, {w, 0}, {w, h}, {0, h}}
		for i= 1, #buttonable_config_info do
			local info= buttonable_config_info[i]
			local name= info[1]
			local button_holder= rec_find_child(button_container, name)
			if button_holder then
				if type(info[2]) == "function" then
					local entry= {name= name, update= info[2]}
					set_or_assert(entry, button_holder, "checkbox", name)
					set_button(entry.checkbox)
					entry.checkbox:playcommand("Set", {value= get_config()[name]})
					clickables[#clickables+1]= entry
				else
					local entry= {
						name= name, inc= info[2], prec= info[3], update= info[4]}
					set_or_assert(entry, button_holder, "increase_button", name)
					set_or_assert(entry, button_holder, "decrease_button", name)
					set_button(entry.increase_button)
					set_button(entry.decrease_button)
					set_or_assert(entry, button_holder, "value_text", name)
					assert(entry.value_text.settext, "value_text actor in " .. name .. " must be a BitmapText")
					set_value_text(entry.value_text, get_config()[name], entry.prec)
					clickables[#clickables+1]= entry
				end
			end
		end
	end,
	LoadActor(clickable_path),
}

local function find_hit_clickable()
	local x= INPUTFILTER:GetMouseX()
	local y= INPUTFILTER:GetMouseY()
	for cid= 1, #clickables do
		local click= clickables[cid]
		if click.checkbox then
			if click.checkbox:pos_in_clickable_area(x, y) then
				return click, click.checkbox
			end
		else
			if click.increase_button:pos_in_clickable_area(x, y) then
				click.increase_button:playcommand("Click")
				return click, 1
			end
			if click.decrease_button:pos_in_clickable_area(x, y) then
				click.decrease_button:playcommand("Click")
				return click, -1
			end
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
				editor_config:set_dirty()
				if clicked.checkbox then
					if button == "left mouse button" then
						clicked.checkbox:playcommand("Click", {value= clicked.update()})
					end
				else
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
					if clicked.update then
						clicked.update()
					end
					set_value_text(clicked.value_text, config[clicked.name], clicked.prec)
				end
			end
		else
			local keymap_entry= keymap[button]
			if keymap_entry then
				editor_config:set_dirty()
				if keymap_entry.amount then
					config[keymap_entry.name]= config[keymap_entry.name] + keymap_entry.amount
					keymap_entry.update()
					if keymap_entry.click then
						set_value_text(keymap_entry.click.value_text, config[keymap_entry.name], keymap_entry.click.prec)
					end
				else
					local value= keymap_entry.update()
					if keymap_entry.click then
						keymap_entry.click.checkbox:playcommand("Click", {value= value})
					end
				end
			end
		end
	else
		if button == config.hide_key then
			toggle_hide()
		end
	end
end

local function make_field(pn)
	return Def.NoteField{
		InitCommand= function(self)
			notefields[pn]= self
			self:hibernate(math.huge)
				:set_vanish_type("FieldVanishType_RelativeToSelf")
				:set_base_values{
					transform_pos_x= _screen.cx, 
					transform_pos_y= _screen.cy,
												}
		end,
	}
end

local old_show_mouse= PREFSMAN:GetPreference("ShowMouseCursor")
local frame= Def.ActorFrame{
	OnCommand= function(self)
		PREFSMAN:SetPreference("ShowMouseCursor", true)
		edit_screen= SCREENMAN:GetTopScreen()
		edit_screen:AddInputCallback(input)
		editor_notefield= edit_screen:GetChild("NoteFieldEdit")
		local skin_name= editor_config:get_data():get_test_skin_choice(get_current_stepstype(stepstype))
		local skin_params= editor_config:get_data():get_skin_params(skin_name)
		for pn, field in pairs(notefields) do
			editor_notefield:share_steps(field)
			field:set_skin(skin_name, skin_params)
			field:set_speed_mod(false, 1)
		end
		self:SetUpdateFunction(update)
		if not get_config().show_preview then
			self:hibernate(math.huge)
		end
		init_keymap()
		reload_mods()
		update_size()
		update_position()
	end,
	NoteskinChangedMessageCommand= function(self)
		local skin_name= editor_config:get_data():get_test_skin_choice(get_current_stepstype(stepstype))
		local skin_params= editor_config:get_data():get_skin_params(skin_name)
		for pn, field in pairs(notefields) do
			if field:get_skin() ~= skin_name then
				field:set_skin(skin_name, skin_params)
			end
		end
	end,
	OffCommand= function(self)
		PREFSMAN:SetPreference("ShowMouseCursor", old_show_mouse)
		editor_config:save()
	end,
	Def.ActorFrame{
		InitCommand= function(self)
			container= self
		end,
		Def.ActorFrameTexture{
			InitCommand= function(self)
				self:setsize(_screen.w, _screen.h)
					:SetTextureName("mod_preview_overlay")
					:EnablePreserveTexture(false)
					:Create()
			end,
			make_field(PLAYER_1),
			make_field(PLAYER_2),
		},
		Def.Sprite{
			Texture= "mod_preview_overlay", InitCommand= function(self)
				preview_sprite= self
			end,
		},
		buttons_frame,
		Def.ActorMultiVertex{
			InitCommand= function(self)
				debug_frames= self
				self:SetDrawState{Mode= "DrawMode_Quads"}
			end,
		},
	},
}

return frame
