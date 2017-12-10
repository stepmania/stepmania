local song_menu_controller= setmetatable({}, menu_controller_mt)
local song_menu_choices

local translation_section= "ScreenEditMenu"

local delayer= false
local delayed_command= {}
local function set_delayed_command(func, desc)
	if delayed_command.func then
		lua.ReportScriptError(tostring(delayed_command.desc) .. " is already in the delayed command slot.  " .. tostring(desc) .. " is trying to replace it.")
		return
	end
	delayed_command.func= func
	delayed_command.desc= desc
	delayer:queuecommand("delayed")
end

local function run_delayed_command()
	if delayed_command.func then
		delayed_command.func()
		delayed_command.func= nil
		delayed_command.desc= nil
	end
end

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

local function dump_steps_info(steps)
	local fields= {
		"GetAuthorCredit", "GetChartStyle", "GetDescription", "GetDifficulty",
		"GetMeter", "GetChartName", "GetStepsType"
	}
	Trace("steps with no difficulty set:")
	for i, field in ipairs(fields) do
		Trace("  " .. field .. ": " .. tostring(steps[field](steps)))
	end
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

-- TODO: Allow picking any stepstype.
local editable_stepstypes= {}
do
	for i, style in ipairs(
		GAMEMAN:GetStylesForGame(GAMESTATE:GetCurrentGame():GetName())) do
		editable_stepstypes[style:GetStepsType()]= true
	end
end

local function handle_edit_setup(song, steps, stype, diff, desc)
	GAMESTATE:JoinPlayer(PLAYER_1)
	local edit_steps= false
	if stype then
		edit_steps= GAMESTATE:SetStepsForEditMode(song, steps, stype, diff, desc)
	else
		edit_steps= GAMESTATE:SetStepsForEditMode(song, steps)
	end
	add_to_recently_edited(song, edit_steps)
	SCREENMAN:GetTopScreen():SetNextScreenName("ScreenEdit")
		:StartTransitioningScreen("SM_GoToNextScreen")
end

local function handle_edit_description_prompt(info)
	local text_settings= {
		Question= THEME:GetString("ScreenEditMenu", "edit_description_prompt"),
		InitialAnswer= "", MaxInputLength= 128,
		OnOK= function(answer)
			if answer ~= "" then
				set_delayed_command(
					function()
						handle_edit_setup(info.song, info.steps, info.stype, "Difficulty_Edit", answer)
					end, "create edit with description")
			end
		end,
		Validate= function(answer, err)
			if SongUtil.validate_edit_description(info.song, info.stype, answer) then
				return true, ""
			else
				return false, THEME:GetString("SongUtil", "The name you chose conflicts with another edit. Please use a different name.")
			end
		end,
	}
	SCREENMAN:AddNewScreenToTop("ScreenTextEntry")
	SCREENMAN:GetTopScreen():Load(text_settings)
end

local function get_open_slots(song)
	local open_slots= {}
	for stype, enabled in pairs(editable_stepstypes) do
		local stype_slot= {}
		for i, diff in ipairs(Difficulty) do
			stype_slot[diff]= true
		end
		open_slots[stype]= stype_slot
	end
	local all_steps= song:get_nonauto_steps()
	for i, steps in ipairs(all_steps) do
		local stype= steps:GetStepsType()
		if editable_stepstypes[stype] then
			local diff= steps:GetDifficulty()
			if diff ~= "Difficulty_Edit" then
				open_slots[stype][diff]= false
			end
		end
	end
	return open_slots
end

local function find_steps_entry_in_song(steps_entry, all_steps)
	for i, steps in ipairs(all_steps) do
		if steps:GetStepsType() == steps_entry.stepstype
			and steps:GetDifficulty() == steps_entry.difficulty
		and steps:GetDescription() == steps_entry.description then
			return steps
		end
	end
end

local short_name_length= 16
local function shorten_name(name)
	return name:sub(1, short_name_length)
end

local steps_list_menu

local function steps_action_menu(info)
	local items= {
		{name= "edit_chart", translation_section= translation_section,
		 type_hint= {main= "action", sub= "edit_chart"},
		 func= function()
			 handle_edit_setup(info.song, info.steps)
		end},
		{name= "delete_chart", translation_section= translation_section,
		 type_hint= {main= "action", sub= "delete_chart"},
		 func= function()
			 prompt_screen(
				 THEME:GetString(translation_section, "delete_chart_prompt"), 1,
				 function(answer)
					 if answer == "yes" then
						 info.song:delete_steps(info.steps)
						 song_menu_controller:handle_menu_action(
							 "close", 1, steps_list_menu(info.song))
					 end
			 end)
		end},
	}
	return nesty_menus.add_close_item(items)
end

local function dest_slot_menu(info)
	local name_format= THEME:GetString(translation_section, info.action.."_format")
	local items= {}
	for i, diff in ipairs(Difficulty) do
		if info.slots[diff] then
			local name= name_format:format(translate_stype(info.stype), translate_diff(diff))
			items[#items+1]= {
				type_hint= {main= "submenu", sub= info.action},
				name= name, dont_translate_name= true, func= function()
					if diff == "Difficulty_Edit" then
						handle_edit_description_prompt(info)
					else
						handle_edit_setup(info.song, info.steps, info.stype, diff)
					end
			end}
		end
	end
	local close= "&leftarrow; " .. THEME:GetString(translation_section, info.action.."_slot_back")
	return nesty_menus.add_close_item(items, close)
end

local function dest_stype_menu(info)
	local items= {}
	local open_slots= get_open_slots(info.song)
	foreach_ordered(
		open_slots, function(stype, slots)
			items[#items+1]= {
				name= translate_stype(stype), dont_translate_name= true,
				type_hint= {main= "submenu", sub= info.action},
				func= function()
					return "submenu", dest_slot_menu{
						song= info.song, steps= info.steps, action= info.action,
						stype= stype, slots= slots}
			end}
	end)
	local close= "&leftarrow; " .. THEME:GetString(translation_section, info.action.."_style_back")
	return nesty_menus.add_close_item(items, close)
end

steps_list_menu= function(song)
	MESSAGEMAN:Broadcast("edit_menu_selection_changed", {song= song})
	local all_steps= song:get_nonauto_steps()
	table.sort(all_steps, steps_compare)
	local items= {}
	for i, steps in ipairs(all_steps) do
		if editable_stepstypes[steps:GetStepsType()] then
			items[#items+1]= {
				name= name_for_steps(steps), dont_translate_name= true,
				type_hint= {main= "submenu"},
				func= function()
					return "submenu", steps_action_menu{song= song, steps= steps}
				end,
				on_focus= function()
					MESSAGEMAN:Broadcast("edit_menu_selection_changed", {steps= steps})
				end,
			}
		end
	end
	items[#items+1]= {
		name= "new_chart", translation_section= translation_section,
		type_hint= {main= "submenu"},
		func= function()
			return "submenu", dest_stype_menu{song= song, action= "new_chart"}
	end}
	items[#items+1]= {
		name= "copy_from", translation_section= translation_section,
		type_hint= {main= "submenu"},
		func= function()
			return "submenu", dest_stype_menu{song= song, action= "copy_from"}
	end}
	local close= "&leftarrow; " .. THEME:GetString(translation_section, "steps_list_back")
	return nesty_menus.add_close_item(items, close)
end

local function song_list_menu(group_name)
	add_to_recent_groups(group_name)
	local songs= SONGMAN:GetSongsInGroup(group_name)
	local items= {}
	for i, song in ipairs(songs) do
		items[#items+1]= {
			name= song:GetDisplayFullTitle(), dont_translate_name= true,
			type_hint= {main= "submenu"},
			func= function()
				return "submenu", steps_list_menu(song)
			end,
			on_focus= function()
				MESSAGEMAN:Broadcast("edit_menu_selection_changed", {song= song})
			end,
		}
	end
	local close= "&leftarrow; " .. THEME:GetString(translation_section, "song_list_back")
	return nesty_menus.add_close_item(items, close)
end

local function recent_edit_menu()
	local function generate()
		local recent= editor_config:get_data().recently_edited
		local items= {}
		for i, entry in ipairs(recent) do
			local song= SONGMAN:find_song_by_dir(entry.song_dir)
			if song then
				local steps= find_steps_entry_in_song(entry, song:get_nonauto_steps())
				if steps then
					items[#items+1]= {
						type_hint= {main= "action", sub= "edit_chart"},
						name= shorten_name(song:GetGroupName()) .. " &leftarrow; " ..
							shorten_name(song:GetDisplayFullTitle()) .. " &leftarrow; " ..
							name_for_steps(steps), dont_translate_name= true,
						func= function()
							handle_edit_setup(song, steps)
						end,
						on_focus= function()
							MESSAGEMAN:Broadcast("edit_menu_selection_changed", {song= song, steps= steps})
						end,
					}
				end
			end
		end
		return nesty_menus.add_close_item(items)
	end
	return {
		type_hint= {main= "submenu"},
		name= "recently_edited", translation_section= translation_section,
		func= function() return "submenu", generate() end}
end

local function recent_group_menu()
	local function generate()
		local recent= editor_config:get_data().recent_groups
		local items= {}
		for i, group_name in ipairs(recent) do
			if SONGMAN:DoesSongGroupExist(group_name) then
				items[#items+1]= {
					name= group_name, dont_translate_name= true,
					type_hint= {main= "submenu"},
					func= function() return "submenu", song_list_menu(group_name) end,
					on_focus= function()
						MESSAGEMAN:Broadcast("edit_menu_selection_changed", {group= group_name})
					end,
				}
			end
		end
		return nesty_menus.add_close_item(items)
	end
	return {
		type_hint= {main= "submenu"},
		name= "recent_groups", translation_section= translation_section,
		func= function() return "submenu", generate() end}
end

local function base_edit_menu()
	local groups= SONGMAN:GetSongGroupNames()
	song_menu_choices= {
		nesty_menus.close_item(THEME:GetString(translation_section, "exit_edit_menu")),
		recent_edit_menu(),
		recent_group_menu(),
	}
	for i, group_name in ipairs(groups) do
		song_menu_choices[#song_menu_choices+1]= {
			name= group_name, dont_translate_name= true,
			type_hint= {main= "submenu"},
			func= function()
				return "submenu", song_list_menu(group_name)
			end,
			on_focus= function()
				MESSAGEMAN:Broadcast("edit_menu_selection_changed", {group= group_name})
			end,
		}
	end
	return song_menu_choices
end

local function exit_edit_menu()
	song_menu_choices= nil
end

local function make_menu_input(sounds)
	return function(event)
		local levels_left, sound_name= song_menu_controller:input(event)
		nesty_menus.play_menu_sound(sounds, sound_name)
		if levels_left and levels_left < 1 then
			editor_config:save()
			SCREENMAN:GetTopScreen():StartTransitioningScreen("SM_GoToPrevScreen")
		end
	end
end

local prev_mx= 0
local prev_my= 0
local buttons_debug= false
local focus_debug= false
local function make_menu_update(sounds)
	return function()
		local mx= INPUTFILTER:GetMouseX()
		local my= INPUTFILTER:GetMouseY()
		if mx ~= prev_mx or my ~= prev_my then
			local sound_name= song_menu_controller:update_focus(mx, my)
			nesty_menus.play_menu_sound(sounds, sound_name)
			prev_mx= mx
			prev_my= my
		end
		if buttons_debug then
			buttons_debug:playcommand("Frame", {song_menu_controller})
		end
		if focus_debug then
			focus_debug:playcommand("Frame", {song_menu_controller})
		end
	end
end

function edit_pick_menu_steps_display_item(
		stype_transform, stype_text, steps_transform, steps_text)
	local steps_item_mt= {
		__index= {
			create_actors= function(self, params)
				return Def.ActorFrame{
					Name= params.name, InitCommand= function(subself)
						self.container= subself
					end,
					steps_text,
				}
			end,
			transform= steps_transform,
			set= function(self, info)
				self.info= into
				if not info then
					self.container:visible(false)
					return
				end
				self.container:visible(true)
				self.container:playcommand("Set", {steps= info})
			end,
	}}
	local stype_item_mt= {
		__index= {
			create_actors= function(self, params)
				self.scroller= setmetatable({disable_wrapping= true}, item_scroller_mt)
				return Def.ActorFrame{
					Name= params.name, InitCommand= function(subself)
						self.container= subself
					end,
					Def.ActorFrame{
						InitCommand= function(subself) self.stype_text= subself end,
						stype_text,
					},
					self.scroller:create_actors(
						"stepstype_display_scroller", 10, steps_item_mt, 0, 0),
				}
			end,
			transform= stype_transform,
			set= function(self, info)
				self.info= info
				if not info then
					self.container:visible(false)
					return
				end
				self.container:visible(true)
				self.stype_text:playcommand("Set", {stype= info.type_name})
				self.scroller:set_info_set(info.steps, 1)
			end,
	}}
	return stype_item_mt
end

function edit_pick_menu_actor(menu_actor, repeats_to_big, debug_click_area)
	repeats_to_big= repeats_to_big or 5
	local frame= Def.ActorFrame{
		Name= "edit_menu",
		OnCommand= function(self)
			local sound_actors= nesty_menus.make_menu_sound_lookup(self)
			SCREENMAN:GetTopScreen():AddInputCallback(make_menu_input(sound_actors))
			self:SetUpdateFunction(make_menu_update(sound_actors))
			song_menu_controller:attach(self:GetChild("menu"))
			song_menu_controller:set_info(base_edit_menu(), true)
			song_menu_controller:set_input_mode("four_direction", repeats_to_big, true)
			song_menu_controller:open_menu()
			if debug_click_area then
				buttons_debug= self:GetChild("buttons_debug")
				focus_debug= self:GetChild("focus_debug")
				local a= .75
				local b= .75 * .25
				local c= .75 * .25 * .25
				song_menu_controller.debug_button_color= {a, b, c, 1}
				song_menu_controller.debug_focus_color= {c, a, b, 1}
				song_menu_controller.debug_scroll_color= {b, c, a, 1}
			end
		end,
		menu_actor,
		Def.Actor{
			InitCommand= function(self)
				delayer= self
			end,
			delayedCommand= function(self)
				if SCREENMAN:GetTopScreen():GetName() == "ScreenEditMenu" then
					run_delayed_command()
				else
					self:sleep(.1):queuecommand("delayed")
				end
			end,
		},
	}
	local sounds= nesty_menus.load_typical_menu_sounds()
	if sounds then
		frame[#frame+1]= sounds
	end
	if debug_click_area then
		frame[#frame+1]= nesty_menus.button_debug_actor()
		frame[#frame+1]= nesty_menus.focus_debug_actor()
	end
	return frame
end

function edit_pick_menu_update_steps_display_info(steps_display)
	return function(self, params)
		if params.group then
			steps_display:set_info_set({}, 1)
		elseif params.song then
			local all_steps= params.song:get_nonauto_steps()
			local by_steps_type= {}
			local type_names= {}
			for i, steps in ipairs(all_steps) do
				local stype= steps:GetStepsType()
				if not by_steps_type[stype] then
					by_steps_type[stype]= {}
					type_names[#type_names+1]= stype
				end
				by_steps_type[stype][#by_steps_type[stype]+1]= steps
			end
			table.sort(type_names)
			local steps_info_set= {}
			for i, name in ipairs(type_names) do
				table.sort(by_steps_type[name], steps_compare)
				steps_info_set[#steps_info_set+1]= {
					type_name= name, steps= by_steps_type[name],
				}
			end
			steps_display:set_info_set(steps_info_set, 1)
		end
	end
end
