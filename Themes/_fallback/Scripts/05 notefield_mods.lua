local function pi_div(equation)
	return {'*', 1/math.pi, equation}
end

local function offset(time, offset)
	if offset == 0 then return time
	else return {'+', time, offset}
	end
end

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
	['sin']= 1,
	['cos']= 1,
	['tan']= 1,
	['square']= 1,
	['triangle']= 1,
	['random']= 1,
	['repeat']= 3,
}

local wave_ops= {
	sin= true, cos= true, tan= true, square= true, triangle= true,
}

local function check_op(eq)
	local op= eq[1]
	local count= arg_counts[op]
	if not count or count+1 == #eq then
-- The mod system's sin/cos functions multiply by pi internally, but these
-- mods ported from ITG weren't made with pi in mind.
-- So divide by pi before sin or cos.
		if wave_ops[op] then
			return pi_div(eq)
		else
			return eq
		end
	end
	assert(false, "Wrong number of operands.  "..op.." requires exactly "..count.." but there are "..(#eq-1))
end

local engine_custom_mods= {
	-- Each entry is a base mod equation and a set of parameters.
	beat= {
		target= 'note_pos_x',
		equation= function(params, ops)
			assert(type(params.width) == "number", "width must be a number.")
			assert(type(params.period) == "number", "period must be a number.")
			local width= params.width
			local width_recip= 1/width
			local period= params.period
			local half_period= period/2
			return check_op{
				ops.level, 20, params.level,
				check_op{
					ops.wave, check_op{
						ops.input, offset(params.input, params.offset), 1/15}},
				{'phase', {'repeat', offset(params.time, params.time_offset), 0, params.period},
				 default= {0, 0, 0, 0},
				 {0, width, width_recip, -1},
				 {half_period-width, half_period, width_recip, 0},
				 {half_period, half_period+width, -width_recip, 1},
				 {period-width, period, -width_recip, 0}
				}
			}
		end,
		params= {
			level= 1, input= 'y_offset', time= 'music_beat', period= 2, width= .2,
			offset= 0, time_offset= 0,
		},
		ops= {
			level= '*', wave= 'cos', input= '*',
		},
		examples= {
			{"beat twice as often", "'beat', {period= 1}"},
			{"beat with longer ramp width", "'beat', {width= 1}"},
		}
	},
	blink= {
		target= 'note_alpha',
		equation= function(params, ops)
			return check_op{
				ops.level,
				check_op{
					ops.round,
					check_op{
						ops.wave,
						check_op{ops.input, offset(params.input, params.offset), 10, params.level}
					},
					params.round,
				},
				1,
			}
		end,
		params= {
			input= 'music_second', level= 1, offset= 0,
		},
		ops= {
			level= '-', round= 'round', wave= 'sin', input= '*',
		},
	},
	bumpy= {
		target= 'note_pos_z',
		equation= function(params, ops)
			return check_op{
				ops.level, 40, params.level,
				check_op{
					ops.wave,
					check_op{ops.input, offset(params.input, params.offset), 1/16},
				},
			}
		end,
		params= {
			level= 1, input= 'y_offset', offset= 0,
		},
		ops= {
			level= '*', wave= 'sin', input= '*',
		},
	},
	confusion= {
		target= 'note_rot_z',
		equation= function(params, ops)
			return check_op{ops.level, offset(params.input, params.offset), params.level, 2*math.pi}
		end,
		params= {
			input= 'music_beat', level= 1, offset= 0,
		},
		ops= {
			level= '*',
		},
	},
	dizzy= {
		target= 'note_rot_z',
		equation= function(params, ops)
			return check_op{ops.level, offset(params.input, params.offset), params.level}
		end,
		params= {
			input= 'dist_beat', level= 1, offset= 0,
		},
		ops= {
			level= '*',
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
	drunk= {
		target= 'note_pos_x',
		equation= function(params, ops)
			return check_op{
				ops.level, params.level, 32,
				check_op{
					ops.wave,
					pi_div{
						ops.time, params.time,
						{ops.column, params.column_input, .2},
						{ops.input, params.input, 10, 1/480},
					},
				},
			}
		end,
		params= {
			level= 1, time= 'music_second', column= 'column', input= 'y_offset',
		},
		ops= {
			level= '*', wave= 'cos', time= '+', column= '*', input= '*',
		},
		examples= {
			{"normal drunk", "'drunk'"},
			{"beat based drunk instead second based drunk", "'drunk', {time= 'music_beat'}"},
			{"tan drunk", "'drunk', {}, {wave= 'tan'}"},
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

local function print_params_info(params)
	if #params == 0 then
		Trace("      None.")
	else
		foreach_ordered(
			params, function(name, value)
				Trace("      "..name .. ", default " .. value)
		end)
	end
end

local function print_examples(examples)
	if not examples or #examples == 0 then
		Trace("      None.")
	else
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
			Trace("    Params:")
			print_params_info(entry.params)
			Trace("    Ops:")
			print_params_info(entry.ops)
			Trace("    Examples:")
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

local function handle_custom_mod(mod_entry)
	local name= mod_entry[1]
	local params= mod_entry[2]
	local ops= mod_entry[3]
	if type(name) == "string" then
		-- Convert this
		-- {start_beat= 0, length_beats= 1, 'beat', {level= 2}, {wave= 'tan'}}
		-- into an equation, using the custom_mods tables.
		local custom_entry= simfile_custom_mods[name] or
			theme_custom_mods[name] or engine_custom_mods[name]
		assert(custom_entry, name .. " was not found in the custom mods list.")
		if type(mod_entry.target) ~= "string" then
			mod_entry.target= custom_entry.target
		end
		if type(mod_entry.sum_type) ~= "string" then
			mod_entry.sum_type= custom_entry.sum_type
		end
		local eq= custom_entry.equation(
			tune_params(params, custom_entry.params),
			tune_params(ops, custom_entry.ops))
		if type(eq) ~= "number" and type(eq) ~= "table" then
			assert(false, "Custom mod " .. name .. " did not return a valid equation.")
		end
		mod_entry[1]= eq
	else
		-- Assume it's already a valid equation, don't change it.
	end
end

local function add_tween_phases(entry)
	if type(entry.on) ~= "number" and type(entry.off) ~= "number" then
		return
	end
	local time= entry.time or 'beat'
	local full_level= 1
	if entry[2] then
		full_level= entry[2].level or full_level
	end
	local phases= {default= {0, 0, 0, 1}}
	if entry.on then
		phases[#phases+1]= {0, entry.on, 1/entry.on, 0}
	end
	if entry.off then
		-- If entry.length is the end of the phase, there is a single frame at
		-- the end of the mod that uses the default phase instead.
		-- I hate floating point math.
		phases[#phases+1]= {entry.length - entry.off, entry.length+.001, -1/entry.off, 1}
	end
	if not entry[2] then
		entry[2]= {}
	end
	entry[2].level= {'*', full_level, {'phase', {'-', 'music_'..time, 'start_'..time}, phases}}
	return true
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
		if entry.field == "all" then
			for fid= 1, num_fields do
				target_fields[#target_fields+1]= result[fid]
			end
		elseif type(entry.field) == "number" then
			if entry.field < 1 or entry.field > num_fields then
				lua.ReportScriptError("mods table entry " .. mid .. " has an invalid field index.")
				return {}
			end
			target_fields[#target_fields+1]= result[fid]
		elseif type(entry.field) == "table" then
			for eid, fid in ipairs(entry.field) do
				if type(fid) ~= "number" or fid < 1 or fid > num_fields then
					lua.ReportScriptError("mods table entry " .. mid .. " has an invalid field index.")
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
		add_tween_phases(entry)
		handle_custom_mod(entry)
		if entry.column then
			local target_columns= {}
			-- Support these kinds of column entries:
			--   column= "all",
			--   column= 1,
			--   column= {2, 3},
			if entry.column == "all" then
				for cid= 1, num_columns do
					target_columns[#target_columns+1]= cid
				end
			elseif type(entry.column) == "number" then
				if entry.column < 1 or entry.column > num_columns then
					lua.ReportScriptError("mods table entry " .. mid .. " has an invalid column index.")
					return {}
				end
				target_columns[#target_columns+1]= entry.column
			elseif type(entry.column) == "table" then
				for eid, cid in ipairs(entry.column) do
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

function organize_and_apply_notefield_mods(notefields, mods)
	local organized_mods= organize_notefield_mods_by_target(mods)
	local pn_to_field_index= PlayerNumber:Reverse()
	if mods.field and mods.field ~= 1 then
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
		end
	end
	organize_and_apply_notefield_mods(notefields, mods)
	return notefields
end

function load_notefield_mods_file(path)
	assert(FILEMAN:DoesFileExist(path))
	local mods, custom_mods= dofile(mods_path)
	set_simfile_custom_mods(custom_mods)
	return handle_notefield_mods(mods)
end
