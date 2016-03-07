-- This contains various functions to make switching to the newfield a bit
-- easier.
-- A theme that uses the newfield should only need to do three things:
-- 1. Add this to the options screen metrics so players can pick a noteskin
--    for the newfield:
--    LineNewSkin="lua,newskin_option_row()"
--    And of course edit the LineNames metric for that screen to include
--    NewSkin.
-- 2. Call use_newfield_on_gameplay during an OnCommand on gameplay.  The
--    argument to use_newfield_on_gameplay is the reverse offset pixels value
--    to use.  If a value is not provied, the default of _screen.h - 64 will
--    be used.  The reverse offset pixels value is the distance from the
--    center of the field to the receptors.
-- use_newfield_on_gameplay will take care of hiding the old notefield and
-- applying the players' speed, perspective, reverse, and mini mods.
-- All other modifiers are currently ignored.  Eventually, conversions for
-- more common modifiers will be written.

-- The new_field branch has a complete rewrite of noteskins, the notefield,
-- and the modifier system.
-- The new noteskin system is documented in NewSkins/default, the example
-- noteskin.
--
-- Currently, both the old notefield and the newfield exist on gameplay.  One
-- of them is hidden by theme code.  By default, the newfield is hidden.
-- When noteskins have been ported and the modifier system completed and edit
-- mode updated, the old notefield will be removed.

-- The functions in this file can be called directly, but calling
-- use_newfield_on_gameplay is simpler and will take care of them.  Only call
-- these functions directly if you need to skip one or more of them.


-- These draw order variables are actually hardcoded in NewField.cpp. They
-- are copied here for convenience.  Do not change them. -Kyz
newfield_draw_order= {
	layer_spacing= 100,
	mid_layer_spacing= 50,
	board= -100,
	mid_board= -50,
	non_board= 0,
	under_field= 50,
	receptor= 100,
	over_receptors= 150,
	hold= 200,
	between_taps_and_holds= 250,
	tap= 300,
	under_explosions= 350,
	explosion= 400,
	over_field= 450,
}

-- The read_bpm arg to set_speed_mod is optional.  If it is exists,
-- the speed is treated as an m-mod.  The read bpm for a chart can be fetched
-- with PlayerState:get_read_bpm() or calculated by the theme.
-- Examples:
--   2x: newfield:set_speed_mod(false, 2)
--   m200: newfield:set_speed_mod(false, 200, read_bpm)
--   c200: newfield:set_speed_mod(true, 200)
function NewField:set_speed_mod(constant, speed, read_bpm)
	if not self then return end
	if not speed then return end
	-- Speed mods are divided by the music rate so that C600 at x2 music rate
	-- is the same as C600 at normal music rate.  If the speed mod was not
	-- divided by the music rate, then a player that reads C600 normally would
	-- have to pick C300 when using x2 music rate.
	local music_rate= GAMESTATE:GetSongOptionsObject("ModsLevel_Current"):MusicRate()
	local mod_input= {}
	local show_unjudgable= true
	local speed_segments_enabled= true
	local scroll_segments_enabled= true
	if constant then
		-- Constant speed mods use the distance in seconds to calculate the arrow
		-- position.
		mod_input= {"ModInputType_DistSecond", (speed / 60) / music_rate}
		-- Hide unjudgable notes so that mines inside of warps are not rendered
		-- on top of the arrows the warp skips to.
		show_unjudgable= false
		-- Disable speed and scroll segments so that they don't affect the speed.
		speed_segments_enabled= false
		scroll_segments_enabled= false
	else
		read_bpm= read_bpm or 1
		-- Non-constant speed mods use the distance in beats to calculate the
		-- arrow position.
		mod_input= {"ModInputType_DistBeat", (speed / read_bpm) / music_rate}
	end
	-- Each column has independent modifier state, so the speed mod needs to be
	-- set in each column.
	for col in ivalues(self:get_columns()) do
		-- The speed modifier is named so that repeated calls to
		-- set_speed_mod do not add stacking speed mods.
		col:get_speed_mod():add_mod{name= "speed", "ModFunctionType_Constant", mod_input}
		col:set_show_unjudgable_notes(show_unjudgable)
		col:set_speed_segments_enabled(speed_segments_enabled)
		col:set_scroll_segments_enabled(scroll_segments_enabled)
	end
end

-- Actually considering implementing a mode switch for the NewField to make
-- it call the ArrowEffects functions, so these oldfield conversion functions
-- are intentionally undocumented.
function NewField:oldfield_tilt(tilt)
	-- The tilt mod is -30 degrees rotation at 1.0.
	local converted_tilt= (tilt * -30) * (math.pi / 180)
	local yoff= 0
	if tilt > 0 then
		yoff= scale(tilt, 0, 1, 0, -45)
	else
		yoff= scale(tilt, 0, -1, 0, -20)
	end
	self:get_trans_rot_x():set_value(converted_tilt)
	self:get_trans_pos_y():add_mod{name= "oldfield_tilt", "ModFunctionType_Constant", yoff}
end

function NewField:oldfield_mini(mini)
	-- mini is zoom 0 at 2.0.
	local converted_zoom= 1 + (mini * -.5)
	for dim in ivalues{"x", "y", "z"} do
		self["get_trans_zoom_"..dim](self):set_value(converted_zoom)
	end
	if math.abs(converted_zoom) < .01 then return end
	local zoom_recip= 1 / converted_zoom
	-- The rev offset values need to be scaled too so the receptors stay fixed.
	for col in ivalues(self:get_columns()) do
		local revoff= col:get_reverse_offset_pixels()
		revoff:set_value(revoff:get_value() * zoom_recip)
		col:set_pixels_visible_after(1024 * zoom_recip)
	end
end

function NewField:oldfield_scrolls(alternate, cross, reverse, split)
	local cols= self:get_columns()
	local before_split= #cols / 2
	local before_cross= #cols / 4
	local after_cross= #cols - before_cross + 1
	for i, col in ipairs(cols) do
		local rev_scale= reverse
		if i > before_split then
			rev_scale= rev_scale + split
		end
		if i % 2 == 0 then
			rev_scale= rev_scale + alternate
		end
		if i > before_cross and i < after_cross then
			rev_scale= rev_scale + cross
		end
		if rev_scale > 2 then
			rev_scale= rev_scale % 2
		end
		if rev_scale > 1 then
			rev_scale= scale(rev_scale, 1, 2, 1, 0)
		end
		rev_scale= scale(rev_scale, 0, 1, 1, -1)
		col:get_reverse_scale():add_mod{name= "old_scrolls", sum_type= "ModSumType_Multiply", "ModFunctionType_Constant", rev_scale}
	end
end

function NewField:set_rev_offset_base(revoff)
	if not revoff then return end
	for col in ivalues(self:get_columns()) do
		col:get_reverse_offset_pixels():set_value(revoff)
	end
end

function NewField:set_reverse_base(rev)
	for col in ivalues(self:get_columns()) do
		col:get_reverse_scale():set_value(rev)
	end
end

function NewField:clear_column_mod(mod_field_name, mod_name)
	for col in ivalues(self:get_columns()) do
		col[mod_field_name](col):remove_mod(mod_name)
	end
end

local function set_alpha_glow_mods(self, alpha_mod, glow_mod)
	for col in ivalues(self:get_columns()) do
		col:get_note_alpha():add_mod(alpha_mod)
		if glow_mod then
			col:get_note_glow():add_mod(glow_mod)
		elseif alpha_mod.name then
			col:get_note_glow():remove_mod(alpha_mod.name)
		end
	end
end

function NewField:set_hidden_mod(line, dist, add_glow)
	local half_dist= dist / 2
	local alpha_mod
	local glow_mod
	if add_glow then
		alpha_mod= {
			name= "hidden", "ModFunctionType_Constant",
			{"ModInputType_YOffset", 1, 0, phases= {
				 default= {0, 0, 0, 0},
				 {-32, line, 0, -1},
			}}
		}
		glow_mod= {
			name= "hidden", "ModFunctionType_Constant",
			{"ModInputType_YOffset", 1, 0, phases= {
				 default= {0, 0, 0, 0},
				 {line - half_dist, line, 1/half_dist, 0},
				 {line, line + half_dist, -1/half_dist, 1},
			}}
		}
	else
		alpha_mod= {
			name= "hidden", "ModFunctionType_Constant",
			{"ModInputType_YOffset", 1, 0, phases= {
				 default= {0, 0, 0, 0},
				 {-32, 0, 0, -1},
				 {0, line - half_dist, 0, -1},
				 {line - half_dist, line + half_dist, 1/dist, -1},
			}}
		}
	end
	set_alpha_glow_mods(self, alpha_mod, glow_mod)
end

function NewField:set_sudden_mod(line, dist, add_glow)
	local half_dist= dist / 2
	local alpha_mod
	local glow_mod
	if add_glow then
		alpha_mod= {
			name= "sudden", "ModFunctionType_Constant",
			{"ModInputType_YOffset", 1, 0, phases= {
				 default= {0, 0, 0, 0},
				 {line, math.huge, 0, -1},
			}}
		}
		glow_mod= {
			name= "sudden", "ModFunctionType_Constant",
			{"ModInputType_YOffset", 1, 0, phases= {
				 default= {0, 0, 0, 0},
				 {line - half_dist, line, 1/half_dist, 0},
				 {line, line + half_dist, -1/half_dist, 1},
			}}
		}
	else
		alpha_mod= {
			name= "sudden", "ModFunctionType_Constant",
			{"ModInputType_YOffset", 1, 0, phases= {
				 default= {0, 0, 0, 0},
				 {line - half_dist, line + half_dist, -1/dist, 0},
				 {line + half_dist, math.huge, 0, -1},
			}}
		}
	end
	set_alpha_glow_mods(self, alpha_mod, glow_mod)
end

function NewField:clear_hidden_mod()
	self:clear_column_mod("get_note_alpha", "hidden")
	self:clear_column_mod("get_note_glow", "hidden")
end

function NewField:clear_sudden_mod()
	self:clear_column_mod("get_note_alpha", "sudden")
	self:clear_column_mod("get_note_glow", "sudden")
end

function find_pactor_in_gameplay(screen_gameplay, pn)
	local pactor= screen_gameplay:GetChild("Player" .. ToEnumShortString(pn))
	if not pactor then
		pactor= screen_gameplay:GetChild("Player")
	end
	return pactor
end

function find_newfield_in_gameplay(screen_gameplay, pn)
	local pactor= find_pactor_in_gameplay(screen_gameplay, pn)
	if not pactor then
		return nil
	end
	return pactor:GetChild("NewField")
end

function convert_oldfield_mods(screen_gameplay, pn, revoff)
	local field= find_newfield_in_gameplay(screen_gameplay, pn)
	if not field then return end
	field:set_rev_offset_base(revoff)
	local pstate= GAMESTATE:GetPlayerState(pn)
	local poptions= pstate:GetPlayerOptions("ModsLevel_Preferred")
	field:oldfield_tilt(poptions:Tilt())
	field:oldfield_mini(poptions:Mini())
	field:oldfield_scrolls(poptions:Alternate(), poptions:Cross(), poptions:Reverse(), poptions:Split())
	local mmod= poptions:MMod()
	local cmod= poptions:CMod()
	local xmod= poptions:XMod()
	if mmod then
		field:set_speed_mod(false, mmod, pstate:get_read_bpm())
	elseif cmod then
		field:set_speed_mod(true, cmod)
	else
		field:set_speed_mod(false, xmod)
	end
end

function use_newfield_on_gameplay()
	local screen_gameplay= SCREENMAN:GetTopScreen()
	if not screen_gameplay.GetLifeMeter then
		lua.ReportScriptError("use_newfield_on_gameplay can only be called when the current screen is ScreenGameplay.")
		return
	end
	for pn in ivalues(GAMESTATE:GetEnabledPlayers()) do
		local pactor= find_pactor_in_gameplay(screen_gameplay, pn)
		if pactor then
			pactor:set_newfield_preferred(true)
			local field= pactor:GetChild("NewField")
			if field then
				apply_newfield_prefs(pn, field, newfield_prefs_config:get_data(pn))
			end
		end
	end
end

function use_newfield_actor()
	return Def.Actor{OnCommand= function(self) use_newfield_on_gameplay() end}
end

function newskin_option_row()
	local pn= GAMESTATE:GetMasterPlayerNumber()
	local steps= GAMESTATE:GetCurrentSteps(pn)
	if not steps then
		steps= GAMESTATE:GetCurrentTrail(pn)
	end
	local stype= false
	if steps then
		stype= steps:GetStepsType()
	elseif not GAMESTATE:InStepEditor() then
		local profile= PROFILEMAN:GetProfile(pn)
		stype= profile:get_last_stepstype()
	end
	if not stype then
		return {
			Name= "NewSkin",
			GoToFirstOnStart= true,
			LayoutType= "ShowAllInRow",
			SelectType= "SelectOne",
			Choices= {""},
			LoadSelections= function() end,
			SaveSelections= function() end,
		}
	end
	local skins= NEWSKIN:get_skin_names_for_stepstype(stype)
	if #skins < 1 then
		return {
			Name= "NewSkin",
			GoToFirstOnStart= true,
			LayoutType= "ShowAllInRow",
			SelectType= "SelectOne",
			Choices= {""},
			LoadSelections= function() end,
			SaveSelections= function() end,
		}
	end
	return {
		Name= "NewSkin",
		GoToFirstOnStart= true,
		LayoutType= "ShowAllInRow",
		SelectType= "SelectOne",
		Choices= skins,
		LoadSelections= function(self, list, pn)
			local player_skin= GAMESTATE:GetPlayerState(pn):GetPlayerOptions("ModsLevel_Preferred"):NewSkin()
			if not GAMESTATE:InStepEditor() then
				local profile= PROFILEMAN:GetProfile(pn)
				player_skin= profile:get_preferred_noteskin(stype)
			end
			for i, choice in ipairs(self.Choices) do
				if player_skin == choice then
					list[i]= true
					return
				end
			end
			list[1]= true
		end,
		SaveSelections= function(self, list, pn)
			for i, choice in ipairs(self.Choices) do
				if list[i] then
					if not GAMESTATE:InStepEditor() then
						local profile= PROFILEMAN:GetProfile(pn)
						local player_skin= profile:set_preferred_noteskin(stype, choice)
					end
					GAMESTATE:GetPlayerState(pn):GetPlayerOptions("ModsLevel_Preferred"):NewSkin(choice)
					return
				end
			end
		end,
	}
end
