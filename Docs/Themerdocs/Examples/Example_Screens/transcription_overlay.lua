-- This is an overlay for adding a transcription window and a mods preview
-- window to edit mode.
-- Put it in the Other folder and use LoadActor to load it in ScreenEdit overlay.lua.
-- 'LoadActor(THEME:GetPathO("", "transcription_overlay.lua"))'

-- It creates a kind of subscreen in the lower left that has a menu for
-- picking a song to display in that window.

-- In the lower right is a window that shows the current chart with mods
-- loaded from 'notefield_mods.lua' (mods_filename) in the song folder.

-- Press 'i' (focus_key) to give the song picking menu focus, or make it lose
-- focus.
-- When the song menu has focus use it to pick a song and chart similarly to
-- the main edit mode menu.
-- Press 'c' (change_chart_key) after a song has been picked to put it back
-- in menu mode to pick another song.  Ignored when menu does not have focus.
-- Because the lower left chart display is meant for transcribing from one
-- chart to another, the chart shown in it will be shown at the current time
-- of the main edit notefield.

-- Press 'z' (mods_reload_key) when the menu does not have focus to reload
-- the mods file so changes are shown in the preview window.

-- That mods file must return a table similar in structure to this:
-- return
-- 	{
-- 		columns= 4,
-- 	{column= 1, target= 'column_pos_y', start_beat= 8.25, length_beats= 2/16,
-- 	 linear_beats_tween(0, 192)},
-- }

-- columns is the number of columns in the chart.
-- Each entry in the table is one mod.
-- If you have a NoteField actor and call 'field:get_mod_target_info()', that
-- function will return a list of valid names for the target field.
-- NoteFieldColumn has its own list of target names.
-- linear_beats_tween is a function written in the mod language that just
-- tweens between two values.

--[[
local function linear_beats_tween(start_mag, end_mag)
	return
		{'+', start_mag,
		 {'*', {'-', end_mag, start_mag},
			{'/',
			 {'-', 'music_beat', 'start_beat'},
			 'length_beats',
			},
		 },
		}
end
]]

-- Writing the lua to load the mods file when the song is played in normal
-- gameplay can be done by copying this code:
--  local mods= lua.load_config_lua(mods_path)
--    Runs the lua file in an empty environment so it can't break or take
--    over the theme.  mods is set to the table the file returns.
--  local organized= organize_notefield_mods_by_target(mods)
--    Takes care of one level of organizing the mods that was easier to write
--    in lua.
--  mods_notefield:clear_timed_mods()
--    Clears mods that were set the previous time the file was loaded.
--  mods_notefield:set_per_column_timed_mods(organized)
--    Loads the mods into the field.



local has_focus= false
local focus_key= "i"
local change_chart_key= "c"
local mods_reload_key= "z"

local transcription_steps= false

local in_menu_mode= true

local notefield= false
local editor_notefield= false
local chart_name= false
local mods_notefield= false

local song_dir= GAMESTATE:GetCurrentSong():GetSongDir()
local mods_filename= "notefield_mods.lua"
local mods_path= song_dir .. mods_filename

local function lose_focus()
	has_focus= false
	SCREENMAN:set_input_redirected(PLAYER_1, false)
end

local song_menu_stack= setmetatable({}, nesty_menu_stack_mt)
local menu_params= {
	name= "menu", x= _screen.cx, y= 0, width= _screen.w*.8,
	num_displays= 1, el_height= 24, display_params= {
		no_status= true,
		height= _screen.h*.8, el_zoom= 1, item_mt= cons_option_item_mt,
		item_params= cons_item_params(),
	},
	cursor_mt= cursor_mt, cursor_params= {
		main= pn_to_color(nil), button_list= button_list_for_menu_cursor(),
		t= .5},
}

local function translate_stype(stype)
	return THEME:GetString("LongStepsType", ToEnumShortString(stype))
end

local function translate_diff(diff)
	return THEME:GetString("CustomDifficulty", ToEnumShortString(diff))
end

local function name_for_steps(steps)
	local name_parts= {
		translate_stype(steps:GetStepsType()),
		translate_diff(steps:GetDifficulty()),
		steps:GetMeter(),
	}
	if steps:GetDifficulty() == "Difficulty_Edit" then
		name_parts[2]= steps:GetDescription()
	end
	return table.concat(name_parts, " ")
end

local steps_compare_fields= {
	"GetMeter", "GetChartStyle", "GetDescription", "GetFilename",
	"GetAuthorCredit", "GetChartName"}
local function steps_compare(left, right)
	if not left then return false end
	if not right then return true end
	local type_diff= StepsType:Compare(left:GetStepsType(), right:GetStepsType())
	if type_diff == 0 then
		local ldiff= left:GetDifficulty()
		local rdiff= right:GetDifficulty()
		if not ldiff then dump_steps_info(left) end
		if not rdiff then dump_steps_info(right) end
		local diff_diff= Difficulty:Compare(left:GetDifficulty(), right:GetDifficulty())
		if diff_diff == 0 then
			for i, comp in ipairs(steps_compare_fields) do
				local ac= left[comp](left)
				local bc= right[comp](right)
				if ac ~= bc then
					return ac < bc
				end
			end
			return false
		else
			return diff_diff < 0
		end
	else
		return type_diff < 0
	end
end

local function generate_steps_list(song)
	local all_steps= song:get_nonauto_steps()
	table.sort(all_steps, steps_compare)
	local choices= {
		name= song:GetDisplayFullTitle(),
		up_text= "&leftarrow; " .. THEME:GetString("ScreenEditMenu", "steps_list_back"),
	}
	for steps in ivalues(all_steps) do
		choices[#choices+1]= {
			name= name_for_steps(steps), translatable= false,
			execute= function()
				song_menu_stack:clear_menu_stack()
				song_menu_stack:hide()
				song_menu_stack.container:hibernate(math.huge)
				transcription_steps= steps
				notefield:set_steps(steps)
				notefield:set_speed_mod(false, 3)
				chart_name:settext(song:GetDisplayFullTitle() .. "\n" .. name_for_steps(steps))
				in_menu_mode= false
				lose_focus()
			end,
		}
	end
	return choices
end

local function generate_song_list(group_name)
	local songs= SONGMAN:GetSongsInGroup(group_name)
	local choices= {
		name= group_name,
		up_text= "&leftarrow; " .. THEME:GetString("ScreenEditMenu", "song_list_back"),
	}
	for song in ivalues(songs) do
		choices[#choices+1]= {
			name= song:GetDisplayFullTitle(), translatable= false,
			menu= nesty_option_menus.menu, args= function()
				return generate_steps_list(song)
			end,
		}
	end
	return choices
end

local song_menu_choices= {}
for group_name in ivalues(SONGMAN:GetSongGroupNames()) do
	song_menu_choices[#song_menu_choices+1]= {
		name= group_name, translatable= false,
		menu= nesty_option_menus.menu, args= function()
			return generate_song_list(group_name)
		end,
	}
end

local paging_buttons= {
	DeviceButton_pgdn= "page_down",
	DeviceButton_pgup= "page_up",
}
local function input(event)
	local button= ToEnumShortString(event.DeviceInput.button)
	if event.type == "InputEventType_FirstPress" and button == mods_reload_key then
		if FILEMAN:DoesFileExist(mods_path) then
			local mods= lua.load_config_lua(mods_path)
			local organized= organize_notefield_mods_by_target(mods)
			mods_notefield:clear_timed_mods()
			mods_notefield:set_per_column_timed_mods(organized)
		end
	end
	if not has_focus then
		if event.type == "InputEventType_FirstPress" and
		button == focus_key then
			has_focus= true
			SCREENMAN:set_input_redirected(PLAYER_1, true)
			return
		end
	else
		if event.type == "InputEventType_FirstPress" and
		button == focus_key then
			lose_focus()
			return
		end
		if event.type == "InputEventType_Release" then return end
		if in_menu_mode then
			local menu_button= paging_buttons[event.DeviceInput.button] or event.GameButton
			if not menu_button or menu_button == "" then return end
			local menu_action= song_menu_stack:interpret_code(menu_button)
			if menu_action == "close" then
				in_menu_mode= false
				notefield:visible(true)
				song_menu_stack:clear_menu_stack()
				song_menu_stack:hide()
				song_menu_stack.container:hibernate(math.huge)
			end
		else
			if button == change_chart_key then
				in_menu_mode= true
				notefield:visible(false)
				song_menu_stack:push_menu_stack(nesty_option_menus.menu, song_menu_choices)
			end
		end
	end
end

local prev_time= -1
local function update(self, delta)
	local time= editor_notefield:get_curr_second()
	mods_notefield:set_curr_second(time)
	if time == prev_time then return end
	prev_time= time
	if transcription_steps then
		notefield:set_curr_second(time)
	end
end

local frame= Def.ActorFrame{
	OnCommand= function(self)
		local screen= SCREENMAN:GetTopScreen()
		screen:AddInputCallback(input)
		song_menu_stack:push_menu_stack(nesty_option_menus.menu, song_menu_choices)
		editor_notefield= screen:GetChild("NoteFieldEdit")
		editor_notefield:share_steps(mods_notefield)
		mods_notefield:set_speed_mod(false, 3)
		self:SetUpdateFunction(update)
	end,
	Def.ActorFrameTexture{
		InitCommand= function(self)
			self:setsize(_screen.w, _screen.h)
				:SetTextureName("transcription_overlay")
				:EnablePreserveTexture(false)
				:Create()
		end,
		song_menu_stack:create_actors(menu_params),
		Def.NoteField{
			InitCommand= function(self)
				notefield= self
				self:set_vanish_type("FieldVanishType_RelativeToSelf")
					:set_skin("default", {})
					:set_base_values{
						transform_pos_x= _screen.cx, 
						transform_pos_y= _screen.cy,
													}
			end,
		},
		Def.BitmapText{
			Font= "Common Normal", InitCommand= function(self)
				chart_name= self
				self:xy(_screen.w * .05, _screen.h * .05):horizalign(left)
			end,
		},
	},
	Def.Sprite{
		Texture= "transcription_overlay", InitCommand= function(self)
			scale_to_fit(self, _screen.w / 3, _screen.h / 3)
			self:xy(0, _screen.h):horizalign(left):vertalign(bottom)
		end,
	},
	Def.ActorFrameTexture{
		InitCommand= function(self)
			self:setsize(_screen.w, _screen.h)
				:SetTextureName("mod_preview_overlay")
				:EnablePreserveTexture(false)
				:Create()
		end,
		Def.NoteField{
			InitCommand= function(self)
				mods_notefield= self
				self:set_vanish_type("FieldVanishType_RelativeToSelf")
					:set_skin("default", {})
					:set_base_values{
						transform_pos_x= _screen.cx, 
						transform_pos_y= _screen.cy,
													}
			end,
		},
		Def.BitmapText{
			Font= "Common Normal", Text= "mods preview",
			InitCommand= function(self)
				self:xy(_screen.w * .05, _screen.h * .05):horizalign(left)
			end,
		},
	},
	Def.Sprite{
		Texture= "mod_preview_overlay", InitCommand= function(self)
			scale_to_fit(self, _screen.w / 3, _screen.h / 3)
			self:xy(_screen.w, _screen.h):horizalign(right):vertalign(bottom)
		end,
	},
}

return frame
