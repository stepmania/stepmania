-- This contains various functions to make switching to the newfield a bit
-- easier.
-- A theme that uses the newfield should only need to do three things:
-- 1. Add this to the options screen metrics so players can pick a noteskin
--    for the newfield:
--    LineNewSkin="lua,noteskin_option_row()"
--    And of course edit the LineNames metric for that screen to include
--    Noteskin.
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
-- The new noteskin system is documented in Noteskins/default, the example
-- noteskin.
--
-- Currently, both the old notefield and the newfield exist on gameplay.  One
-- of them is hidden by theme code.  By default, the newfield is hidden.
-- When noteskins have been ported and the modifier system completed and edit
-- mode updated, the old notefield will be removed.

-- The functions in this file can be called directly, but calling
-- use_newfield_on_gameplay is simpler and will take care of them.  Only call
-- these functions directly if you need to skip one or more of them.


-- These draw order variables are actually hardcoded in NoteField.cpp. They
-- are copied here for convenience.  Do not change them. -Kyz
notefield_draw_order= {
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
--   2x: notefield:set_speed_mod(false, 2)
--   m200: notefield:set_speed_mod(false, 200, read_bpm)
--   c200: notefield:set_speed_mod(true, 200)
function NoteField:set_speed_mod(constant, speed, read_bpm)
	if not self then return end
	if not speed then return end
	-- Speed mods are divided by the music rate so that C600 at x2 music rate
	-- is the same as C600 at normal music rate.  If the speed mod was not
	-- divided by the music rate, then a player that reads C600 normally would
	-- have to pick C300 when using x2 music rate.
	-- The mod input music rate is used so that the speed mod will react when
	-- the music rate changes during the song.
	local mod_input= "dist_beat"
	local mod_mult= 1
	local show_unjudgable= true
	local speed_segments_enabled= true
	local scroll_segments_enabled= true
	if constant then
		-- Constant speed mods use the distance in seconds to calculate the arrow
		-- position.
		mod_input= "dist_second"
		mod_mult= {"/", (speed / 60), "music_rate"}
		-- Hide unjudgable notes so that mines inside of warps are not rendered
		-- on top of the arrows the warp skips to.
		show_unjudgable= false
		-- Disable speed and scroll segments so that they don't affect the speed.
		speed_segments_enabled= false
		scroll_segments_enabled= false
	else
		-- Non-constant speed mods use the distance in beats to calculate the
		-- arrow position.
		mod_input= "dist_beat"
		if read_bpm then
			mod_mult= {"/", (speed / read_bpm), "music_rate"}
		else
			mod_mult= speed
		end
	end
	-- Each column has independent modifier state, so the speed mod needs to be
	-- set in each column.
	local speed_mod= {target= "speed", name= "speed", {"*", mod_input, mod_mult}}
	for col in ivalues(self:get_columns()) do
		-- The speed modifier is named so that repeated calls to
		-- set_speed_mod do not add stacking speed mods.
		col:set_permanent_mods{speed_mod}
		col:set_show_unjudgable_notes(show_unjudgable)
		col:set_speed_segments_enabled(speed_segments_enabled)
		col:set_scroll_segments_enabled(scroll_segments_enabled)
	end
	return self
end

function NoteField:set_rev_offset_base(revoff)
	if not revoff then return end
	for col in ivalues(self:get_columns()) do
		col:set_base_values{{target= "revserse_offset", value= revoff}}
	end
	return self
end

function NoteField:set_reverse_base(rev)
	for col in ivalues(self:get_columns()) do
		col:set_base_values{{target= "revserse", value= rev}}
	end
	return self
end

function NoteField:set_hidden_mod(line, dist, add_glow)
	local half_dist= dist / 2
	if add_glow then
		local mods= {
			{
				target= "note_alpha", name= "hidden", {
					"phase", "y_offset", {default= {0, 0, 0, 0}, {-32, line, 0, -1},
			}}},
			{
				target= "note_glow", name= "hidden", {
					"phase", "y_offset", {default= {0, 0, 0, 0},
						{line - half_dist, line, 1/half_dist, 0},
						{line, line + half_dist, -1/half_dist, 1},
			}}},
		}
		for col in ivalues(self:get_columns()) do
			col:set_permanent_mods(mods)
		end
	else
		local mods= {
			{
			target= "note_alpha", name= "hidden", {"phase", "y_offset", {
				default= {0, 0, 0, 0},
				 {-32, 0, 0, -1},
				 {0, line - half_dist, 0, -1},
				 {line - half_dist, line + half_dist, 1/dist, -1},
			}}},
		}
		for col in ivalues(self:get_columns()) do
			col:set_permanent_mods(mods)
			col:remove_permanent_mods{{target= "note_glow", name= "hidden"}}
		end
	end
	return self
end

function NoteField:set_sudden_mod(line, dist, add_glow)
	local half_dist= dist / 2
	if add_glow then
		local mods= {
		{
			target= "note_alpha", name= "sudden", {"phase", "y_offset", {
				 default= {0, 0, 0, 0},
				 {line, math.huge, 0, -1},
		}}},
		{
			target= "note_glow", name= "sudden", {"phase", "y_offset", {
				 default= {0, 0, 0, 0},
				 {line - half_dist, line, 1/half_dist, 0},
				 {line, line + half_dist, -1/half_dist, 1},
		}}},
		}
		for col in ivalues(self:get_columns()) do
			col:set_permanent_mods(mods)
		end
	else
		local mods= {
		{
			target= "note_alpha", name= "sudden", {"phase", "y_offset", {
				 default= {0, 0, 0, 0},
				 {line - half_dist, line + half_dist, -1/dist, 0},
				 {line + half_dist, math.huge, 0, -1},
		}}},
		}
		for col in ivalues(self:get_columns()) do
			col:set_permanent_mods(mods)
			col:remove_permanent_mods{{target= "note_glow", name= "sudden"}}
		end
	end
	return self
end

function NoteField:clear_hidden_mod()
	for col in ivalues(self:get_columns()) do
		col:remove_permanent_mods{
			{target= "note_glow", name= "hidden"},
			{target= "note_alpha", name= "hidden"},
		}
	end
	return self
end

function NoteField:clear_sudden_mod()
	for col in ivalues(self:get_columns()) do
		col:remove_permanent_mods{
			{target= "note_glow", name= "sudden"},
			{target= "note_alpha", name= "sudden"},
		}
	end
	return self
end

function find_pactor_in_gameplay(screen_gameplay, pn)
	local pactor= screen_gameplay:GetChild("Player" .. ToEnumShortString(pn))
	if not pactor then
		pactor= screen_gameplay:GetChild("Player")
	end
	return pactor
end

function find_notefield_in_gameplay(screen_gameplay, pn)
	local pactor= find_pactor_in_gameplay(screen_gameplay, pn)
	if not pactor then
		return nil
	end
	return pactor:GetChild("NoteField")
end

function oitg_zoom_mode_actor()
	return Def.Actor{
		OnCommand= function(self)
			local screen= SCREENMAN:GetTopScreen()
			for i, pn in ipairs(GAMESTATE:GetEnabledPlayers()) do
				local field= find_notefield_in_gameplay(screen, pn)
				if field then
					field:set_oitg_zoom_mode(true)
				end
			end
		end,
	}
end
