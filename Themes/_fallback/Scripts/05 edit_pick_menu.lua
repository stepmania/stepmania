local song_menu_stack= setmetatable({}, nesty_menu_stack_mt)
local song_menu_choices

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
	if stype then
		GAMESTATE:SetStepsForEditMode(song, steps, stype, diff, desc)
	else
		GAMESTATE:SetStepsForEditMode(song, steps)
	end
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
	}
	SCREENMAN:AddNewScreenToTop("ScreenTextEntry")
	SCREENMAN:GetTopScreen():Load(text_settings)
end

local function generate_steps_action(info)
	return {
		name= name_for_steps(info.steps),
		up_text= "&leftarrow; " .. THEME:GetString("ScreenEditMenu", "steps_action_back"),
		{
			name= "edit_chart", translatable= true,
			execute= function()
				handle_edit_setup(info.song, info.steps)
			end,
		},
		{
			name= "delete_chart", translatable= true,
			execute= function()
				prompt_screen(
					THEME:GetString("ScreenEditMenu", "delete_chart_prompt"), 1,
					function(answer)
						if answer == "yes" then
							info.song:delete_steps(info.steps)
						end
						song_menu_stack:pop_menu_stack()
				end)
			end,
		},
	}
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
	local all_steps= song:GetAllSteps()
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

local function generate_copy_dest_slot_menu(info)
	local choices= {
		name= "copy_dest_slot_menu",
		up_text= "&leftarrow; " .. THEME:GetString("ScreenEditMenu", "copy_dest_slot_back"),
	}
	for i, diff in ipairs(Difficulty) do
		if info.slots[diff] then
			local name_format= THEME:GetString("ScreenEditMenu", "copy_to_slot_format")
			local name= name_format:format(translate_stype(info.stype), translate_diff(diff))
			choices[#choices+1]= {
				name= name, translatable= false,
				execute= function()
					if diff == "Difficulty_Edit" then
						handle_edit_description_prompt(info)
					else
						handle_edit_setup(info.song, info.steps, info.stype, diff)
					end
				end,
			}
		end
	end
	return choices
end

local function generate_copy_dest_menu(info)
	local choices= {
		name= "copy_dest_stype_menu",
		up_text= "&leftarrow; " .. THEME:GetString("ScreenEditMenu", "copy_dest_style_back"),
	}
	local open_slots= get_open_slots(info.song)
	foreach_ordered(
		open_slots, function(stype, slots)
			choices[#choices+1]= {
				name= translate_stype(stype), translatable= false,
				menu= nesty_option_menus.menu, args= function()
					return generate_copy_dest_slot_menu{
						song= info.song, steps= info.steps, stype= stype, slots= slots}
				end,
			}
	end)
	return choices
end

local function generate_copy_menu(info)
	local choices= {
		name= "copy_from_menu",
		up_text= "&leftarrow; " .. THEME:GetString("ScreenEditMenu", "copy_from_back"),
	}
	local copyable_steps= {}
	local all_steps= info.song:GetAllSteps()
	table.sort(all_steps, steps_compare)
	for i, steps in ipairs(all_steps) do
		choices[#choices+1]= {
			name= name_for_steps(steps), translatable= false,
			menu= nesty_option_menus.menu, args= function()
				return generate_copy_dest_menu{song= info.song, steps= steps}
			end,
		}
	end
	return choices
end

local function generate_new_chart_slot_menu(info)
	local choices= {
		name= "new_chart_slot_menu",
		up_text= "&leftarrow; " .. THEME:GetString("ScreenEditMenu", "new_chart_slot_back"),
	}
	for i, diff in ipairs(Difficulty) do
		if info.slots[diff] then
			local name_format= THEME:GetString("ScreenEditMenu", "new_chart_slot_format")
			local name= name_format:format(translate_stype(info.stype), translate_diff(diff))
			choices[#choices+1]= {
				name= name, translatable= false, execute= function()
					if diff == "Difficulty_Edit" then
						handle_edit_description_prompt(info)
					else
						handle_edit_setup(info.song, info.steps, info.stype, diff)
					end
				end,
			}
		end
	end
	return choices
end

local function generate_new_chart_stype_menu(info) 
	local choices= {
		name= "new_chart_stype_menu",
		up_text= "&leftarrow; " .. THEME:GetString("ScreenEditMenu", "new_chart_stype_back"),
	}
	local open_slots= get_open_slots(info.song)
	foreach_ordered(
		open_slots, function(stype, slots)
			choices[#choices+1]= {
				name= translate_stype(stype), translatable= false,
				menu= nesty_option_menus.menu, args= function()
					return generate_new_chart_slot_menu{
						song= info.song, stype= stype, slots= slots}
				end,
			}
	end)
	return choices
end

local function generate_steps_list(song)
	MESSAGEMAN:Broadcast("edit_menu_selection_changed", {song= song})
	local all_steps= song:GetAllSteps()
	local choices= {
		name= song:GetDisplayFullTitle(),
		recall_init_on_pop= true,
		up_text= "&leftarrow; " .. THEME:GetString("ScreenEditMenu", "steps_list_back"),
	}
	for i, steps in ipairs(all_steps) do
		if editable_stepstypes[steps:GetStepsType()] then
			choices[#choices+1]= {
				name= name_for_steps(steps), translatable= false,
				menu= nesty_option_menus.menu, args= function()
					return generate_steps_action{song= song, steps= steps}
				end,
				on_focus= function()
					MESSAGEMAN:Broadcast("edit_menu_selection_changed", {steps= steps})
				end,
			}
		end
	end
	choices[#choices+1]= {
		name= "new_chart", translatable= true,
		menu= nesty_option_menus.menu, args= function()
			return generate_new_chart_stype_menu{song= song}
		end,
	}
	choices[#choices+1]= {
		name= "copy_from", translatable= true,
		menu= nesty_option_menus.menu, args= function()
			return generate_copy_menu{song= song}
		end,
	}
	return choices
end

local function generate_song_list(group_name)
	local songs= SONGMAN:GetSongsInGroup(group_name)
	local choices= {
		name= group_name,
		up_text= "&leftarrow; " .. THEME:GetString("ScreenEditMenu", "song_list_back"),
	}
	for i, song in ipairs(songs) do
		choices[#choices+1]= {
			name= song:GetDisplayFullTitle(), translatable= false,
			menu= nesty_option_menus.menu, args= function()
				return generate_steps_list(song)
			end,
			on_focus= function()
				MESSAGEMAN:Broadcast("edit_menu_selection_changed", {song= song})
			end,
		}
	end
	return choices
end

local function init_edit_picker()
	local groups= SONGMAN:GetSongGroupNames()
	song_menu_choices= {}
	for i, group_name in ipairs(groups) do
		song_menu_choices[#song_menu_choices+1]= {
			name= group_name, translatable= false,
			menu= nesty_option_menus.menu, args= function()
				return generate_song_list(group_name)
			end,
			on_focus= function()
				MESSAGEMAN:Broadcast("edit_menu_selection_changed", {group= group_name})
			end,
		}
	end
end

local function exit_edit_picker()
	song_menu_choices= nil
end

local paging_buttons= {
	DeviceButton_pgdn= "page_down",
	DeviceButton_pgup= "page_up",
}

local function menu_input(event)
	if event.type == "InputEventType_Release" then return end
	local button= paging_buttons[event.DeviceInput.button] or event.GameButton
	if not button or button == "" then return end
	local menu_action= song_menu_stack:interpret_code(button)
	if menu_action == "close" then
		lua.ReportScriptError("lol no")
	else
		
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

function edit_pick_menu_actor(menu_params)
	menu_params.translation_section= "ScreenEditMenu"
	return Def.ActorFrame{
		OnCommand= function(self)
			SCREENMAN:GetTopScreen():AddInputCallback(menu_input)
			init_edit_picker()
			song_menu_stack:push_menu_stack(nesty_option_menus.menu, song_menu_choices, THEME:GetString("ScreenEditMenu", "exit_edit_menu"))
			song_menu_stack:update_cursor_pos()
		end,
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
		song_menu_stack:create_actors(menu_params),
	}
end

function edit_pick_menu_update_steps_display_info(steps_display)
	return function(self, params)
		if params.group then
			steps_display:set_info_set({}, 1)
		elseif params.song then
			local all_steps= params.song:GetAllSteps()
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
