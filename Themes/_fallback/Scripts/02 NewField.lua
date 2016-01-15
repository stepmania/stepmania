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

-- The read_bpm arg to set_newfield_speed_mod is optional.  If it is exists,
-- the speed is treated as an m-mod.  The read bpm for a chart can be fetched
-- from with PlayerState:get_read_bpm() or calculated by the theme.
-- Examples:
--   2x: set_newfield_speed_mod(newfield, false, 2)
--   m200: set_newfield_speed_mod(newfield, false, 200, read_bpm)
--   c200: set_newfield_speed_mod(newfield, true, 200)
function set_newfield_speed_mod(newfield, constant, speed, read_bpm)
	if not newfield then return end
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
	for col in ivalues(newfield:get_columns()) do
		-- The speed modifier is named so that repeated calls to
		-- set_newfield_speed_mod do not add stacking speed mods.
		col:get_speed_mod():add_mod{name= "speed", "ModFunctionType_Constant", mod_input}
		col:set_show_unjudgable_notes(show_unjudgable)
		col:set_speed_segments_enabled(speed_segments_enabled)
		col:set_scroll_segments_enabled(scroll_segments_enabled)
	end
end

function set_newfield_tilt(newfield, tilt)
	-- The tilt mod is -30 degrees rotation at 1.0.
	local converted_tilt= (tilt * -30) * (math.pi / 180)
	newfield:get_trans_rot_x():set_value(converted_tilt)
end

function set_newfield_mini(newfield, mini)
	-- mini is zoom 0 at 2.0.
	local converted_zoom= 1 + (mini * -.5)
	for dim in ivalues{"x", "y", "z"} do
		newfield["get_trans_zoom_"..dim](newfield):set_value(converted_zoom)
	end
	if math.abs(converted_zoom) < .01 then return end
	local zoom_recip= 1 / converted_zoom
	-- The rev offset values need to be scaled too so the receptors stay fixed.
	for col in ivalues(newfield:get_columns()) do
		local revoff= col:get_reverse_offset_pixels()
		revoff:set_value(revoff:get_value() * zoom_recip)
		col:set_pixels_visible_after(1024 * zoom_recip)
	end
end

function set_newfield_rev_offset(newfield, revoff)
	if not revoff then return end
	for col in ivalues(newfield:get_columns()) do
		col:get_reverse_offset_pixels():set_value(revoff)
	end
end

function set_newfield_reverse(newfield, rev)
	for col in ivalues(newfield:get_columns()) do
		col:get_reverse_scale():set_value(rev)
	end
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

function set_newfield_mods(screen_gameplay, pn, revoff)
	local field= find_newfield_in_gameplay(screen_gameplay, pn)
	if not field then return end
	set_newfield_rev_offset(field, revoff)
	local pstate= GAMESTATE:GetPlayerState(pn)
	local poptions= pstate:GetPlayerOptions("ModsLevel_Preferred")
	set_newfield_tilt(field, poptions:Tilt())
	set_newfield_mini(field, poptions:Mini())
	set_newfield_reverse(field, 1-(2*poptions:Reverse()))
	local mmod= poptions:MMod()
	local cmod= poptions:CMod()
	local xmod= poptions:XMod()
	if mmod then
		set_newfield_speed_mod(field, false, mmod, pstate:get_read_bpm())
	elseif cmod then
		set_newfield_speed_mod(field, true, cmod)
	else
		set_newfield_speed_mod(field, false, xmod)
	end
end

function use_newfield_on_gameplay(revoff)
	local screen_gameplay= SCREENMAN:GetTopScreen()
	if not screen_gameplay.GetLifeMeter then
		lua.ReportScriptError("use_newfield_on_gameplay can only be called when the current screen is ScreenGameplay.")
		return
	end
	for pn in ivalues(GAMESTATE:GetEnabledPlayers()) do
		local pactor= find_pactor_in_gameplay(screen_gameplay, pn)
		pactor:set_newfield_preferred(true)
		set_newfield_mods(screen_gameplay, pn, revoff)
	end
end

function use_newfield_actor(revoff)
	return Def.Actor{OnCommand= function(self) use_newfield_on_gameplay(revoff) end}
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
