-- These mods are not identical to ITG's.
-- Any mods that repeat something repeat either every 2 beats or every 2
-- arrow heights.
-- Wave motions range from -32 to +32, or from 0 to +64.

local operator_list= ModValue.get_operator_list()
local mod_input_names= ModValue.get_input_list()

-- operators are considered interchangeable if they have the same number of
-- operands.
local arg_counts= {
	['%']= 2,
	['mod']= 2,
	['o']= 2,
	['round']= 2,
	['_']= 2,
	['floor']= 2,
	['ceil']= 2,
	['|']= 1,
	['abs']= 1,
	['sin']= 1,
	['cos']= 1,
	['tan']= 1,
	['square']= 1,
	['triangle']= 1,
	['asin']= 1,
	['acos']= 1,
	['atan']= 1,
	['asquare']= 1,
	['atriangle']= 1,
	['random']= 1,
	['repeat']= 3,
}

local function check_op(eq)
	local op= eq[1]
	local count= arg_counts[op]
	if not count or count+1 == #eq then
		return eq
	end
	assert(false, "Wrong number of operands.  "..op.." requires exactly "..count.." but there are "..(#eq-1))
end

local function scale_equation(thing, from_low, from_high, to_low, to_high)
	local from_size= {'-', from_high, from_low}
	local to_size= {'-', to_high, to_low}
	local scale= {'/', to_size, from_size}
	return {'+', to_low, {'*', scale, {'-', thing, from_low}}}
end

local engine_custom_mods= {
	-- Each entry is a base mod equation and a set of parameters.
	time_based_wave= {
		target= 'note_pos_x',
		equation= function(params, ops)
			local wave= check_op{ops.wave, params.input}
			return {'*', 32, params.level, wave}
		end,
		params= {
			level= 1, input= 'music_beat',
		},
		ops= {
			wave= 'sin',
		},
		examples= {
		},
	},
	height_based_wave= {
		target= 'note_pos_x',
		equation= function(params, ops)
			local note_input= {'*', params.input, 1/64}
			local wave= check_op{ops.wave, note_input}
			return {'*', 32, params.level, wave}
		end,
		params= {
			level= 1, input= 'y_offset',
		},
		ops= {
			wave= 'sin',
		},
		examples= {
		},
	},
	attenuate= {
		target= 'note_pos_x',
		equation= function(params, ops)
			local col_x= {'*', {'-', params.column, {'*', params.num_columns, .5}, -.5}, params.column_spacing}
			local adj_xoff= {'*', col_x, 1/64}
			local adj_yoff= {'*', params.input, 1/64}
			return {ops.level, params.level, adj_yoff, adj_yoff, adj_xoff}
		end,
		params= {
			level= 1, num_columns= 4, input= 'y_offset', column= 'column', column_spacing= 64
		},
		ops= {
			level= '*',
		},
		examples= {
			{"attenuate x", "'attenuate'"},
			{"attenuate y", "target= 'note_pos_y', 'attenuate'"},
			{"attenuate z", "target= 'note_pos_z', 'attenuate'"},
			{"attenuate in double", "'attenuate', {num_columns= 8}"},
			{"attenuate as if in a different column", "'attenuate', {column= {'+', 'column', {'sin', 'music_beat'}}}"},
		},
	},
	beat= {
		target= 'note_pos_x',
		equation= function(params, ops)
			-- triangle wave - n results in the section above zero having width 1-n
			-- so triangle - (1-w) will give us width w
			-- params.time is doubled and offset to put a peak on every beat.
			local triangle= {'triangle', {'+', .5, {'*', 2, params.time}}}
			local above_zero_is_width= {'-', triangle, {'-', 1, params.width}}
			-- height above zero needs to be 1, but is currently w.
			-- <result> / w will make the height 1
			local height_is_one= {'/', above_zero_is_width, params.width}
			-- then max is used to clip off the parts of the wave that are below
			-- zero, and we have a wave that is flat between peaks.
			local beat_wave= {'max', 0, height_is_one}
			local curved_wave= {'^', beat_wave, 2}
			-- inverter has peaks on even beats, and troughs on odd beats, to make
			-- the motion alternate directions.
			local inverter= {'square', {'+', .5, params.time}}
			local time_beat_factor= {'*', curved_wave, 32, inverter}
			local note_beat_factor= {
				ops.wave,
				{'+', params.wave_offset,
				 {'*', params.input, 1/64},
				},
			}
			local time_and_beat= check_op{ops.factor, time_beat_factor, note_beat_factor}
			return check_op{ops.level, params.level, time_and_beat}
		end,
		params= {
			level= 1, input= 'y_offset', wave_offset= .5, period= 1,
			time= 'music_beat', width= .5,
		},
		ops= {
			level= '*', factor= '*', wave= 'sin',
		},
		examples= {
			{"beat twice as hard", "'beat', {level= {mult= 2}}"},
			{"beat twice as often", "'beat', {time= {mult= 2}}"},
			{"beat with longer ramp time", "'beat', {width= {mult= 2}}"},
		}
	},
	blink= {
		target= 'note_alpha',
		equation= function(params, ops)
			return check_op{
				ops.offset,
				check_op{
					ops.round,
					check_op{
						ops.wave,
						check_op{ops.input, params.input, params.level}
					},
					params.round,
				},
				params.offset,
			}
		end,
		params= {
			input= 'music_second', level= 1, round= 1/3, offset= 0,
		},
		ops= {
			offset= '-', round= 'round', wave= 'sin', input= '*',
		},
		examples= {
			{"blink on beat of instead second", "'blink', {input= 'music_beat'}"},
			{"only go half-transparent", "'blink', {offset= -.5}"},
		},
	},
	bounce= {
		target= 'note_pos_x',
		equation= function(params, ops)
			local wave_input= {'*', params.input, 1/64}
			local wave= check_op{ops.wave, wave_input}
			local amount= {'|', wave}
			return {'*', params.level, 64, .5, amount}
		end,
		params= {
			level= 1, input= 'y_offset',
		},
		ops= {
			wave= 'sin',
		},
		examples= {
		},
	},
	bumpy= {
		target= 'note_pos_z',
		redir= 'height_based_wave',
	},
	confusion= {
		target= 'note_rot_z',
		equation= function(params, ops)
			return check_op{ops.level, params.input, params.level}
		end,
		params= {
			input= 'music_beat', level= 1, offset= 0,
		},
		ops= {
			level= '*',
		},
		examples= {
			{"y_offset confusion instead of beat", "'confusion', {input= 'y_offset'}"},
		},
	},
	digital= {
		target= 'note_pos_x',
		equation= function(params, ops)
			local note_input= {'*', params.input, 1/64}
			local note_wave= check_op{ops.wave, note_input}
			local stepped_wave= check_op{ops.round, note_wave, {'/', 1, params.steps}}
			return {'*', params.level, 32, stepped_wave}
		end,
		params= {
			level= 1, steps= 1, input= 'y_offset',
		},
		ops= {
			wave= 'sin', round= 'o',
		},
		examples= {
		},
	},
	dizzy= {
		target= 'note_rot_z',
		redir= 'confusion',
		params= {
			input= 'dist_beat'
		},
		examples= {
			{"music beat dizzy instead of dist beat", "'dizzy', {input= 'music_beat'}"},
		},
	},
	dizzyh= {
		target= 'note_rot_z',
		redir= 'roll',
		examples= {
		},
	},
	drunk= {
		target= 'note_pos_x',
		equation= function(params, ops)
			local note_input= {'*', params.input, 1/64}
			local column_input= {'*', params.column, 1/8}
			local wave_input= {'+', params.time, note_input, column_input}
			local wave= check_op{ops.wave, wave_input}
			return {'*', params.level, 32, wave}
		end,
		params= {
			level= 1, time= 'music_second', column= 'column', input= 'y_offset',
		},
		ops= {
			wave= 'cos',
		},
		examples= {
			{"normal drunk", "'drunk'"},
			{"beat based drunk instead second based drunk", "'drunk', {time= 'music_beat'}"},
			{"tan drunk", "'drunk', {}, {wave= 'tan'}"},
		},
	},
	flip= {
		target= 'column_pos_x',
		sum_type= '*',
		equation= function(params, ops)
			return {'*', -1, params.level}
		end,
		params= {
			level= 1,
		},
		ops= {
		},
		examples= {
		},
	},
	invert= {
		target= 'column_pos_x',
		equation= function(params, ops)
			-- Use wave functions to generate {1, -1, 1, -1, ...}
			local offset_dir= {ops.wave, {'+', params.column, .5}}
			return {'*', offset_dir, params.spacing, params.level}
		end,
		params= {
			level= 1, column= 'column', spacing= 64,
		},
		ops= {
			wave= 'square',
		},
		examples= {
		},
	},
	mini= {
		target= 'transform_zoom',
		sum_type= '-',
		equation= function(params, ops)
			return {'*', params.level, .5}
		end,
		params= {
			level= 1,
		},
		ops= {
		},
		examples= {
			{"mini is not per-column", "field= 'all', 'mini'"},
		},
	},
	parabola= {
		target= 'note_pos_x',
		equation= function(params, ops)
			local scaled_input= check_op{ops.scale, params.input, params.spacing}
			local raised_input= check_op{ops.raise, scaled_input, params.power}
			return check_op{ops.level, params.level, raised_input}
		end,
		params= {
			level= 1, input= 'y_offset', power= 2, spacing= 64,
		},
		ops= {
			scale= '/', raise= '^', level= '*',
		},
		examples= {
		},
	},
	pulse= {
		target= 'note_zoom',
		sum_type= '*',
		equation= function(params, ops)
			local note_input= {'*', params.input, 1/64}
			local wave= check_op{ops.wave, note_input}
			local amplitude= {'-', params.outer, params.inner}
			local scaled_wave= {'*', wave, amplitude, params.level}
			local shifted_wave= {'+', scaled_wave, params.inner, {'*', amplitude, 1/2}}
			return shifted_wave
		end,
		params= {
			level= 1, input= 'y_offset', outer= 1.5, inner= .5,
		},
		ops= {
			wave= 'sin',
		},
		examples= {
		},
	},
	roll= {
		target= 'note_rot_x',
		equation= function(params, ops)
			return {'*', params.level, params.input, 1/64}
		end,
		params= {
			level= 1, input= 'y_offset',
		},
		ops= {
		},
		examples= {
		},
	},
	sawtooth= {
		target= 'note_pos_x',
		equation= function(params, ops)
			-- changing the low and high marks of the wave shouldn't move the peaks
			-- normal wave input has the range 0-2, default low and high is .5-1
			-- so dividing by 4 would put the peaks as far apart as a normal wave
			-- If the operands in input_scale look backwards, remember this:
			--   a / (b/c) = a * (c/b)
			local input_scale= {'/', {'-', params.high, params.low}, 2}
			local note_input= {'*', params.input, 1/64, input_scale}
			local sawed_input= {'repeat', note_input, params.low, params.high}
			local wave= check_op{ops.wave, sawed_input}
			return {'*', 32, params.level, wave}
		end,
		params= {
			level= 1, input= 'y_offset', low= .5, high= 1,
		},
		ops= {
			wave= 'triangle',
		},
		examples= {
		},
	},
	shrinklinear= {
		target= 'note_zoom',
		equation= function(params, ops)
			-- assuming sum_type is add, multiplying the main result by 0
			-- when input is less than 0 will disable it.
			local disable_past_receptors_phases= {default= {0, 0, 0, 1}, {-1000000, 0, 0, 0}}
			local main_equation= {'*', params.input, {'*', .5, params.level, 1/64}}
			return {'*', main_equation, {'phase', params.input, disable_past_receptors_phases}}
		end,
		params= {
			level= 1, input= 'y_offset',
		},
		ops= {
			level= '-',
		},
		examples= {
			{"normal shrinkage", "'shrink_to_linear'"},
			{"I was in the pool!", "'shrink_to_linear', 10"},
			{"beat shrinkage", "'shrink_to_linear', {input= 'dist_beat'}"},
		},
	},
	shrinkmult= {
		target= 'note_zoom',
		sum_type= '*',
		equation= function(params, ops)
			-- Cap input at 0 to disable the effect after the receptors.
			return {'/', 1, {'+', 1, {'*', {'max', 0, params.input}, params.level, 1/100}}}
		end,
		params= {
			level= 1, input= 'y_offset',
		},
		ops= {
			level= '-',
		},
		examples= {
			{"normal shrinkage", "'shrink_to_mult'"},
			{"I was in the pool!", "'shrink_to_mult', 10"},
			{"beat shrinkage", "'shrink_to_mult', {input= 'dist_beat'}"},
		},
	},
	square= {
		target= 'note_pos_x',
		redir= 'height_based_wave',
		ops= {
			wave= 'square',
		},
		examples= {
		},
	},
	stealth= {
		target= 'note_alpha',
		equation= function(params, ops)
			return {ops.level, params.offset, params.level}
		end,
		params= {
			level= 1, offset= 1,
		},
		ops= {
			level= '-',
		},
	},
	tiny= {
		target= 'note_zoom',
		sum_type= '*',
		equation= function(params, ops)
			return {'^', .5, params.level}
		end,
		params= {
			level= 1,
		},
		ops= {
		},
		examples= {
		},
	},
	tipsy= {
		target= 'column_pos_y',
		equation= function(params, ops)
			local column= {'*', params.column, .5}
			local wave_input= {'+', params.time, column}
			local wave= check_op{ops.wave, wave_input}
			return {'*', params.level, wave, 32}
		end,
		params= {
			level= 1, time= 'music_second', column= 'column',
		},
		ops= {
			wave= 'cos',
		},
		examples= {
		},
	},
	tornado= {
		target= 'note_pos_x',
		equation= function(params, ops)
			local left= {'max', 0, {'-', params.column, params.width}}
			local right= {'min', {'-', params.num_columns, 1}, {'+', params.column, params.width}}
			local amplitude= {'-', right, left}
			local arc_wave_input= scale_equation(params.column, left, right, -1, 1)
			local arc_wave= check_op{ops.arc_wave, arc_wave_input}
			local note_input= {'*', params.input, 1/64}
			local wave_input= {'+', arc_wave, note_input}
			local wave= check_op{ops.wave, wave_input}
			local shifted_wave= scale_equation(wave, -1, 1, left, right)
			local shift_amount= {'-', shifted_wave, params.column}
			return {'*', shift_amount, params.level, 64}
		end,
		params= {
			level= 1, width= 3, num_columns= 4, column= 'column', input= 'y_offset',
		},
		ops= {
			arc_wave= 'acos', wave= 'cos',
		},
		examples= {
		},
	},
	twirl= {
		target= 'note_rot_y',
		redir= 'roll',
		examples= {
		},
	},
	xmode= {
		target= 'note_pos_x',
		equation= function(params, ops)
			-- use a square wave that repeats every (num_columns / num_pads) to
			-- set the direction.
			local cols_per_wave= {'/', params.num_columns, params.num_pads}
			local wave_input= {'/', params.column, cols_per_wave, .5}
			local offset_dir= check_op{ops.wave, wave_input}
			return {'*', params.level, offset_dir, params.y_offset}
		end,
		params= {
			level= 1, num_columns= 4, num_pads= 1, column= 'column', y_offset= 'y_offset',
		},
		ops= {
			wave= 'square',
		},
		examples= {
		},
	},
	zigzag= {
		target= 'note_pos_x',
		redir= 'height_based_wave',
		ops= {
			wave= 'triangle',
		},
		examples= {
		},
	},
}

local function tune_params(params, defaults)
	if type(params) == "table" then
		add_defaults_to_params(params, defaults)
		return params
	else
		return defaults
	end
end

local function sanity_check_custom_mods(mods)
	for name, mod in pairs(mods) do
		add_blank_tables_to_params(mod, {"params", "ops", "examples"})
	end
end

local theme_custom_mods= {}

function set_theme_custom_mods(mods)
	if type(mods) ~= "table" then mods= {} end
	theme_custom_mods= mods
end

local simfile_custom_mods= {}

function set_simfile_custom_mods(mods)
	if type(mods) ~= "table" then mods= {} end
	simfile_custom_mods= mods
end

local function print_params_info(name, params)
	if not params then
		Trace("    "..name..":  None.")
		return
	end
	local has_none= true
	for name, meh in pairs(params) do
		has_none= false
		break
	end
	if has_none then
		Trace("    "..name..":  None.")
	else
		Trace("    "..name..":")
		foreach_ordered(
			params, function(name, value)
				Trace("      "..name .. ", default " .. value)
		end)
	end
end

local function print_examples(examples)
	if not examples or #examples == 0 then
		Trace("    Examples:  None.")
	else
		Trace("    Examples:")
		for exid= 1, #examples do
			local entry= examples[exid]
			Trace("      " .. entry[1])
			Trace("        " .. entry[2])
		end
	end
end

local function print_info_from_custom_mods(mods)
	foreach_ordered(
		mods, function(name, entry)
			Trace("  "..name .. ", " .. entry.target)
			if entry.redir then
				Trace("    This is a redir to " .. entry.redir)
			end
			print_params_info("Params", entry.params)
			print_params_info("Ops", entry.ops)
			print_examples(entry.examples)
	end)
end

function print_custom_mods_info()
	Trace("Engine custom mods:")
	print_info_from_custom_mods(engine_custom_mods)
	Trace("Theme custom mods:")
	print_info_from_custom_mods(theme_custom_mods)
	Trace("Simfile custom mods:")
	print_info_from_custom_mods(simfile_custom_mods)
	assert(false, "CUPS is not correctly configured for HP LaserJet 2700")
end

local function custom_mods_has_list(mods)
	local list= {}
	for name, enter in pairs(mods) do
		list[name]= true
	end
	return list
end

function get_custom_mods_list()
	return {
		engine= custom_mods_has_list(engine_custom_mods),
		theme= custom_mods_has_list(theme_custom_mods),
		simfile= custom_mods_has_list(simfile_custom_mods),
	}
end

local function is_an_equation(eq)
	local equation_op= eq[1]
	if equation_op and operator_list[equation_op] then
		local might_be_spline= equation_op == "spline"
		for key, value in pairs(eq) do
			if type(key) == "string" then
				if might_be_spline and (key == "loop" or key == "polygonal") then
					-- do nothing
				else
					return false
				end
			end
		end
	else
		return false
	end
	return true
end

local function tween_mod_param(full, on, off, length, time, start)
	if time ~= "second" and time ~= "beat" then
		time= "beat"
	end
	local phases= {default= {0, 0, 0, 1}}
	if type(on) == "number" and on ~= 0 then
		phases[#phases+1]= {0, on, 1/on, 0}
	end
	if type(off) == "number" and off ~= 0 then
		-- If entry.length is the end of the phase, there is a single frame at
		-- the end of the mod that uses the default phase instead.
		-- I hate floating point math.
		phases[#phases+1]= {length - off, length+.001, -1/off, 1}
	end
	if #phases == 0 then
		return full
	end
	local phase_eq= {'phase', {'-', 'music_'..time, 'start_'..time}, phases}
	if start then
		return {'+', start, {'*', {'-', full, start}, phase_eq}}
	else
		return {'*', full, phase_eq}
	end
end

local function parse_param_string(str)
	-- "v1 +2 *3"
	-- "1 +2 *3"
	-- to
	-- {value= 1, add= 2, mult= 3}
	local parts= split(" ", str)
	local first_to_field= {
		v= "value",
		['+']= "add",
		['*']= "mult",
	}
	local ret= {}
	for i= 1, #parts do
		local part= parts[i]
		local field_name= first_to_field[part:sub(1, 1)]
		if field_name then
			ret[field_name]= tonumber(part:sub(2, -1))
		else
			ret.value= tonumber(part)
		end
	end
	return ret
end

local function maybe_tween_thing(mod_entry, thing, from, is_level)
	if mod_entry.on or mod_entry.off and
	(is_level or type(thing) == "number") then
		return tween_mod_param(thing, mod_entry.on, mod_entry.off, mod_entry.length, mod_entry.time, from)
	else
		return thing
	end
end

local function process_param(mod_entry, param, default, is_level)
	local value= param.value or param.v
	local mult= param.mult or param.m
	local add= param.add or param.a

	local tween_from= default
	if is_level then
		tween_from= 0
	end
	local value_eq= maybe_tween_thing(mod_entry, value or default, tween_from, is_level)
	local mult_eq= maybe_tween_thing(mod_entry, mult, 1, is_level)
	local add_eq= maybe_tween_thing(mod_entry, add, 0, is_level)
	local partial_eq= value_eq
	if mult then
		partial_eq= {'*', partial_eq, mult_eq}
	end
	if add then
		partial_eq= {'+', partial_eq, add_eq}
	end
	return partial_eq
end

local function combine_params(redir, base)
	if type(redir) ~= "table" then
		return base
	end
	local ret= {}
	for name, base_param in pairs(base) do
		local redir_param= redir[name]
		if redir_param then
			local redir_type= type(redir_param)
			if redir_type == "string" then
				if mod_input_names[redir_param] then
					redir_param= {value= redir_param}
				else
					redir_param= parse_param_string(redir_param)
				end
			elseif redir_type == "number" then
				redir_param= {value= redir_param}
			end
			local partial_eq= redir_param.value or redir_param.v or base_param
			local mult_eq= redir_param.mult or redir_param.m
			local add_eq= redir_param.add or redir_param.a
			if mult_eq then
				partial_eq= {'*', partial_eq, mult_eq}
			end
			if add_eq then
				partial_eq= {'+', partial_eq, add_eq}
			end
			ret[name]= partial_eq
		else
			ret[name]= base_param
		end
	end
	return ret
end

local function combine_ops(dom, sub)
	if type(dom) ~= "table" then
		return sub
	end
	local ret= {}
	for name, sub_op in pairs(sub) do
		ret[name]= dom[name] or sub_op
	end
	return ret
end

local function lookup_mod(name)
	return simfile_custom_mods[name] or theme_custom_mods[name] or
		engine_custom_mods[name]
end

local function resolve_redir(name, redir_stack)
	local entry= lookup_mod(name)
	assert(entry, name .. " was not found in the custom mods list.")
	assert(entry.equation or entry.redir, name .. " is not a valid entry in the custom mods list.")
	if entry.equation then
		return entry
	end
	if entry.redir then
		if #redir_stack > 64 then
			Trace("custom mod redir stack:")
			for i= 1, #redir_stack do
				Trace("  " .. redir_stack[i])
			end
			assert(false, "Too many custom mod redirs in chain, or redir loop.")
		end
		redir_stack[#redir_stack+1]= name
		local sub= resolve_redir(entry.redir, redir_stack)
		return {
			target= entry.target or sub.target,
			sum_type= entry.sum_type or sub.sum_type,
			equation= sub.equation,
			params= combine_params(entry.params, sub.params),
			ops= combine_ops(entry.ops, sub.ops),
		}
	end
end

function find_custom_mod(name)
	return resolve_redir(name, {})
end

function handle_custom_mod(mod_entry)
	local name= mod_entry[1]
	local params= mod_entry[2]
	local ops= mod_entry[3]
	if type(name) == "string" then
		mod_entry.time= mod_entry.time or 'beat'
		-- Convert this
		-- {start_beat= 0, length_beats= 1, 'beat', {level= 2}, {wave= 'tan'}}
		-- into an equation, using the custom_mods tables.
		local custom_entry= resolve_redir(name, {})
		assert(custom_entry, name .. " was not found in the custom mods list.")
		if type(params) == "number" then
			params= {level= params}
		elseif type(params) == "table" then
			if is_an_equation(params) then
				params= {level= params}
			end
		else
			params= {}
		end
		if not params.level then
			params.level= custom_entry.params.level
		end
		if type(mod_entry.target) ~= "string" then
			mod_entry.target= custom_entry.target
		end
		if type(mod_entry.sum_type) ~= "string" then
			mod_entry.sum_type= custom_entry.sum_type
		end
		local processed_params= {}
		local params_type= type(params)
		if params_type == "nil" then
			processed_params.level= mod_entry.params.level
		elseif params_type == "boolean" then
			processed_params.level= params
		elseif params_type == "string" then
			if mod_input_names[params] then
				processed_params.level= params
			else
				processed_params.level=
					process_param(
						mod_entry, parse_param_string(params),
						custom_entry.params.level, true)
			end
		elseif params_type == "number" then
			processed_params.level= tween_mod_param(
				params, mod_entry.on, mod_entry.off, mod_entry.length, mod_entry.time)
		elseif params_type == "table" then
			if is_an_equation(params) then
				processed_params.level= tween_mod_param(
					params, mod_entry.on, mod_entry.off, mod_entry.length, mod_entry.time)
			else
				for name, param in pairs(params) do
					local ptype= type(param)
					if ptype == "boolean" then
						processed_params[name]= param
					elseif ptype == "string" then
						if mod_input_names[param] then
							processed_params[name]= param
						else
							processed_params[name]= process_param(
								mod_entry, parse_param_string(param),
								custom_entry.params[name], name == "level")
						end
					elseif ptype == "number" then
						local from= custom_entry.params[name]
						if name == "level" then
							from= 0
						end
						processed_params[name]= maybe_tween_thing(mod_entry, param, from)
					elseif ptype == "table" then
						if is_an_equation(param) then
							processed_params[name]= param
						else
							processed_params[name]= process_param(
								mod_entry, param, custom_entry.params[name], name == "level")
						end
					end
				end
			end
		end
		local eq= custom_entry.equation(
			add_defaults_to_params(processed_params, custom_entry.params),
			add_defaults_to_params(ops, custom_entry.ops))
		if type(eq) ~= "number" and type(eq) ~= "table" then
			assert(false, "Custom mod " .. name .. " did not return a valid equation.")
		end
		mod_entry[1]= eq
	else
		if mod_entry.on or mod_entry.off then
			mod_entry[1]= tween_mod_param(mod_entry[1], mod_entry.on, mod_entry.off, mod_entry.length, mod_entry.time)
		end
	end
end

function organize_notefield_mods_by_target(mods_table)
	local num_fields= mods_table.fields
	if type(num_fields) ~= "number" or num_fields < 1 then
		num_fields= 1
	end
	local num_columns= mods_table.columns
	if type(num_columns) ~= "number" or num_columns < 1 then
		lua.ReportScriptError("organize_notefield_mods_by_target cannot correctly handle mods that target all columns when the mods table does not specify the number of columns.")
		return {}
	end
	local result= {}
	for fid= 1, num_fields do
		local field_result= {field= {}}
		for cid= 1, num_columns do
			field_result[cid]= {}
		end
		result[fid]= field_result
	end
	for mid= 1, #mods_table do
		local entry= mods_table[mid]
		if type(entry) ~= "table" then
			lua.ReportScriptError("mods table entry " .. mid .. " is not a table.")
			return {}
		end
		local target_fields= {}
		-- Support these kinds of field entries:
		--   field= "all",
		--   field= 1,
		--   field= {2, 3},
		if entry.field == "all" or entry.field == nil then
			for fid= 1, num_fields do
				target_fields[#target_fields+1]= result[fid]
			end
		elseif type(entry.field) == "number" then
			if entry.field < 1 or entry.field > num_fields then
				lua.ReportScriptError("mods table entry " .. mid .. " field index " .. entry.field .. " is more than " .. num_fields)
				return {}
			end
			target_fields[#target_fields+1]= result[entry.field]
		elseif type(entry.field) == "table" then
			for eid, fid in ipairs(entry.field) do
				if type(fid) ~= "number" or fid < 1 or fid > num_fields then
					lua.ReportScriptError("mods table entry " .. mid .. " field index " .. entry.field .. " is more than " .. num_fields)
					return {}
				end
				target_fields[#target_fields+1]= result[fid]
			end
		elseif type(entry.field) ~= "nil" then
			lua.ReportScriptError("mods table entry " .. mid .. " has an invalid field index.")
			return {}
		else
			target_fields[#target_fields+1]= result[1]
		end
		handle_custom_mod(entry)
		if entry.column then
			local column= entry.column
			if type(column) == "function" then
				column= column(num_columns)
			end
			local target_columns= {}
			-- Support these kinds of column entries:
			--   column= "all",
			--   column= 1,
			--   column= {2, 3},
			if column == "all" then
				for cid= 1, num_columns do
					target_columns[#target_columns+1]= cid
				end
			elseif type(column) == "number" then
				if column < 1 or column > num_columns then
					lua.ReportScriptError("mods table entry " .. mid .. " has an invalid column index.")
					return {}
				end
				target_columns[#target_columns+1]= column
			elseif type(column) == "table" then
				for eid, cid in ipairs(column) do
					if type(cid) ~= "number" or cid < 1 or cid > num_columns then
						lua.ReportScriptError("mods table entry " .. mid .. " has an invalid column index.")
						return {}
					end
					target_columns[#target_columns+1]= cid
				end
			else
				lua.ReportScriptError("mods table entry " .. mid .. " has an invalid column index.")
				return {}
			end
			for i, targ_field in ipairs(target_fields) do
				for i, cid in ipairs(target_columns) do
					local targ_column= targ_field[cid]
					targ_column[#targ_column+1]= entry
				end
			end
		else
			for i, targ_field in ipairs(target_fields) do
				targ_field.field[#targ_field.field+1]= entry
			end
		end
	end
	if num_fields == 1 then
		return result[1]
	else
		return result
	end
end

function process_mod_skin_entries(notefields, noteskins)
	local profiles= {}
	for pn, field in pairs(notefields) do
		profiles[pn]= PROFILEMAN:GetProfile(pn)
	end
	local function get_params_for_skin(pn, skin, maybe)
		if type(maybe) == "table" then return maybe end
		return profiles[pn]:get_noteskin_params(skin)
	end
	local function set_for_both(skin)
		for pn, field in pairs(notefields) do
			field:set_skin(skin, get_params_for_skin(pn, skin))
		end
	end
	-- noteskin= "lambda"
	if type(noteskins) == "string" then
		set_for_both(noteskins)
		return
	end
	if type(noteskins) ~= "table" then return end
	if #noteskins < 1 then return end
	-- noteskin= {
	--   -- If add is true, skins are loaded after the one the player picked.
	--   -- Otherwise, first entry replaces noteskin from the player's profile.
	--   add= true,
	--   -- Affects all notefields, uses params from profiles.
	--   {name= "lambda"},
	--   -- Affects all notefields, uses params provided.
	--   {name= "lambda", field= "all", params= {}},
	--   -- Affects player 1 notefield, uses params from profile.
	--   {name= "lambda", field= 1},
	--   -- Affects player 2 notefield, uses params from profile.
	--   {name= "lambda", field= 2},
	--   -- Affects player 1 and 2 notefields, uses params from profiles.
	--   {name= "lambda", field= {1, 2}},
	--   -- Choose two random noteskins and apply them.
	--   {random= 2, field= "all"},
	--   -- Choose two random non-generic noteskins and apply them.
	--   {random= 2, field= "all", disable_supports_all= true},
	-- }
	local set_flags= {}
	if noteskins.add then
		for pn, field in pairs(notefields) do
			set_flags[pn]= true
		end
	end
	local function do_set(pn, skin, params)
		params= get_params_for_skin(pn, skin, params)
		if set_flags[pn] then
			notefields[pn]:add_skin(skin, params)
		else
			notefields[pn]:set_skin(skin, params)
			set_flags[pn]= true
		end
	end
	local function get_skins(pn, stype, without, nofilter)
		local skins= NOTESKIN:get_skin_names_for_stepstype(stype, without)
		if not nofilter then
			skins= filter_noteskin_list_with_shown_config(pn, skins)
		end
		for i, name in ipairs(skins) do
			skins[name]= true
		end
		return skins
	end
	local all_skin_lists= {}
	for pn, field in pairs(notefields) do
		local stype= field:get_stepstype()
		all_skin_lists[pn]= {
			unfiltered= get_skins(pn, stype, false, true),
			with_all= get_skins(pn, stype, false),
			without_all= get_skins(pn, stype, true),
		}
	end
	local function add_pick(pick, have)
		have[#have+1]= pick
		have[pick]= true
	end
	local function get_list_for_random(pn, disable_supports_all)
		if disable_supports_all
		and #all_skin_lists[pn].without_all > 0 then
			return all_skin_lists[pn].without_all
		end
		return all_skin_lists[pn].with_all
	end
	local function pick_random(list, already_have)
		if #list == 1 then
			add_pick(list[1], already_have)
			return
		end
		local name
		repeat
			local pick= math.random(1, #list)
			name= list[pick]
			local used= already_have[name]
		until not used or #list <= #already_have
		add_pick(name, already_have)
	end
	local function set_picks(fields, picks)
		for pn, field in pairs(fields) do
			for i, pick in ipairs(picks) do
				do_set(pn, pick)
			end
		end
	end
	for i, entry in ipairs(noteskins) do
		if type(entry) == "table" then
			local fields= {}
			local targype= type(entry.field)
			if targype == "number" then
				local pn= PlayerNumber[entry.field] or PlayerNumber[1]
				fields= {[pn]= notefields[pn]}
			elseif targype == "table" then
				for tid, dy in ipairs(entry.field) do
					local pn= PlayerNumber[dy] or PlayerNumber[1]
					fields= {[pn]= notefields[pn]}
				end
			else
				fields= notefields
			end
			if entry.random == "all" then
				local first_pn= next(fields)
				local picks= DeepCopy(get_list_for_random(first_pn, entry.disable_supports_all))
				local len= #picks
				for to= 1, len-1 do
					local from= math.random(to, len)
					picks[to], picks[from]= picks[from], picks[to]
				end
				set_picks(fields, picks)
			elseif type(entry.random) == "number" then
				local first_pn= next(fields)
				local list= get_list_for_random(first_pn, entry.disable_supports_all)
				local picks= {}
				for pid= 1, entry.random do
					pick_random(list, picks)
				end
				set_picks(fields, picks)
			else
				local skin= entry.name
				for pn, field in pairs(fields) do
					if all_skin_lists[pn].unfiltered[skin] then
						do_set(pn, skin, entry.params)
					end
				end
			end
		end
	end
end

local pn_to_field_index= PlayerNumber:Reverse()

function organize_and_apply_notefield_mods(notefields, mods)
	local first_pn, first_field= next(notefields, nil)
	mods.columns= first_field:get_num_columns()
	process_mod_skin_entries(notefields, mods.noteskin)
	mods.noteskin= nil
	local organized_mods= organize_notefield_mods_by_target(mods)
	if mods.fields and mods.fields ~= 1 then
		for pn, field in pairs(notefields) do
			-- stepmania enums are 0 indexed
			local field_index= pn_to_field_index[pn] + 1
			field:set_per_column_timed_mods(organized_mods[field_index])
		end
	else
		for pn, field in pairs(notefields) do
			field:set_per_column_timed_mods(organized_mods)
		end
	end
end

function handle_notefield_mods(mods)
	local notefields= {}
	local screen= SCREENMAN:GetTopScreen()
	if screen.GetEditState then
		-- edit mode
		notefields[PLAYER_1]= screen:GetChild("Player"):GetChild("NoteField")
	else
		-- gameplay
		for i, pn in ipairs(GAMESTATE:GetEnabledPlayers()) do
			notefields[pn]= find_notefield_in_gameplay(screen, pn)
			if notefields[pn] then
				notefields[pn]:clear_timed_mods()
				notefields[pn]:clear_to_base_skin()
			end
		end
	end
	if type(mods) ~= "table" then return notefields end
	organize_and_apply_notefield_mods(notefields, mods)
	return notefields
end

function load_notefield_mods_file(path)
	assert(FILEMAN:DoesFileExist(path))
	local mods, custom_mods= dofile(path)
	set_simfile_custom_mods(custom_mods)
	return handle_notefield_mods(mods)
end
