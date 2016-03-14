local newfield_default_prefs= {
	speed_mod= 250,
	speed_type= "maximum",
	hidden= false,
	hidden_offset= 120,
	sudden= false,
	sudden_offset= 190,
	fade_dist= 40,
	glow_during_fade= true,
	fov= 45,
	reverse= 1,
	rotation_x= 0,
	rotation_y= 0,
	rotation_z= 0,
	vanish_x= 0,
	vanish_y= 0,
	yoffset= 130,
	zoom= 1,
	zoom_x= 1,
	zoom_y= 1,
	zoom_z= 1,
}

newfield_speed_types= {"maximum", "constant", "multiple"}

-- If the theme author uses Ctrl+F2 to reload scripts, the config that was
-- loaded from the player's profile will not be reloaded.
-- But the old instance of newfield_prefs_config still exists, so the data
-- from it can be copied over.  The config system has a function for handling
-- this.
newfield_prefs_config= create_lua_config{
	name= "newfield_prefs", file= "newfield_prefs.lua",
	default= newfield_default_prefs,
	-- use_alternate_config_prefix is meant for lua configs that are shared
	-- between multiple themes.  It should be nil for preferences that will
	-- only exist in your theme. -Kyz
	use_alternate_config_prefix= "",
}

add_standard_lua_config_save_load_hooks(newfield_prefs_config)

function set_newfield_default_yoffset(yoff)
	newfield_default_prefs.yoffset= yoff
end

function apply_newfield_prefs(pn, field, prefs)
	local torad= math.pi / 180
	local pstate= GAMESTATE:GetPlayerState(pn)
	if prefs.speed_type == "maximum" then
		field:set_speed_mod(false, prefs.speed_mod, pstate:get_read_bpm())
	elseif prefs.speed_type == "constant" then
		field:set_speed_mod(true, prefs.speed_mod)
	else
		field:set_speed_mod(false, prefs.speed_mod)
	end
	field:get_fov_mod():set_value(prefs.fov)
	field:get_vanish_x_mod():set_value(prefs.vanish_x)
	field:get_vanish_y_mod():set_value(prefs.vanish_y)
	field:get_trans_rot_x():set_value(prefs.rotation_x*torad)
	field:get_trans_rot_y():set_value(prefs.rotation_y*torad)
	field:get_trans_rot_z():set_value(prefs.rotation_z*torad)
	-- Use the y zoom to adjust the y offset to put the receptors in the same
	-- place.
	local adjusted_offset= prefs.yoffset / (prefs.zoom * prefs.zoom_y)
	for i, col in ipairs(field:get_columns()) do
		col:get_reverse_scale():set_value(prefs.reverse)
		col:get_reverse_offset_pixels():set_value(adjusted_offset)
	end
	if prefs.hidden then
		field:set_hidden_mod(prefs.hidden_offset, prefs.fade_dist, prefs.glow_during_fade)
	else
		field:clear_hidden_mod()
	end
	if prefs.sudden then
		field:set_sudden_mod(prefs.sudden_offset, prefs.fade_dist, prefs.glow_during_fade)
	else
		field:clear_sudden_mod()
	end
	field:get_trans_zoom_x():set_value(prefs.zoom * prefs.zoom_x)
	field:get_trans_zoom_y():set_value(prefs.zoom * prefs.zoom_y)
	field:get_trans_zoom_z():set_value(prefs.zoom * prefs.zoom_z)
end
