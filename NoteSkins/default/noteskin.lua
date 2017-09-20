return {
	-- notes is the file to use to load the note columns, the taps and holds.
	notes= "notes.lua",
	-- layers is a list of files to load things in the columns that are
	-- displayed in the columns.  Typically, this is just the receptors and
	-- explosions.  The layers are rendered by draw order.
	-- Docs/Themerdocs/5.1_incompatibilities/NewField.md explains draw orders.
	-- receptors.lua has the explanation of the requirements of a layer file.
	layers= {"receptors.lua", "explosions.lua"},
	-- A noteskin may optionally have layers that are rendered in the field,
	-- instead of part of each column.
	--field_layers= {"calipers"},
	-- Since all layers are considered the same, messages such as
	-- judgment and step actions are sent to all layers.  This means you can
	-- make receptors that respond to judgments, or explosions that respond to
	-- steps, or whatever.
	supports_all_buttons= false,
	buttons= {"Left", "Down", "Up", "Right", "UpLeft", "UpRight", "DownLeft", "DownRight"},
	-- The fallback entry is optional.  It can be used to name another noteskin
	-- to fall back on if a hold texture or one of the files listed above is
	-- not found.
	fallback= "",
	-- These are example player colors for multiplayer mask mode.
	-- Any number of player colors can be specified and the notefield will use
	-- the ones needed.
	player_colors= {{.8, 0, 0, 1}, {0, 0, .8, 1}, {.8, .8, 0, 1},
		{.4, 0, .8, 1}, {.8, 0, .8, 1}, {.8, .4, 0, 1}},
	-- The skin_parameters table is an optional way to make the noteskin
	-- customizable.  skin_parameters can be a table that contains any numbers,
	-- strings, or bools that the noteskin author wants.  The third parameter
	-- to the loading functions for the notes and layers will be a lua table
	-- with the exact same structure.  If you look in receptors.lua in this
	-- noteskin, you can see it using skin_parameters.receptors.warning_time to
	-- set a warning_time variable.
	-- The engine takes care of sanity checking the skin_parameters table, so
	-- the noteskin author doesn't have to check for a field being empty or
	-- some nonsense.
	skin_parameters= {
		quantize_holds= false,
		explosions= {
			particles= true,
			particle_dist= 128,
			particle_life= 1,
			particle_size= 32,
			num_particles= 16,
			particle_blend= "BlendMode_WeightedMultiply",
		},
		receptors= {
			warning_time= 2,
		},
	},
	-- If you use skin_parameters, you must also supply the skin_parameter_info
	-- table.  skin_parameter_info contains all the info the engine needs for
	-- sanity checking, and the info the theme needs for building a menu.
	-- For every element of the skin_parameters table, there must be a matching
	-- table in the skin_parameter_info table describing it.  If there isn't a
	-- matching table, the theme won't be able to make menu options for it.
	skin_parameter_info= {
		quantize_holds= {
			translation= {
				en= {title= "Quantize holds", explanation= "When on, holds will be quantized."},
			},
		},
		explosions= {
			-- The translation table provides the strings the theme displays in the
			-- menu for the option.  Each element in the translation table is for
			-- one language.  The same two-letter language codes used by normal
			-- language files are used here.  So the "en" entry is for english,
			-- "ja" would be for japanese.
			translation= {
				en= {title= "Explosions", explanation= "Options for the explosion effects."},
			},
			particles= {
				translation= {
					en= {title= "Particles", explanation= "When set to true, particles will fly out when a note is hit."},
			}},
			particle_dist= {
				-- If the option is a number, the type field should be provided to
				-- tell the theme what kind of number it is.  If type is "int", the
				-- theme knows to provide an integer number.  If type is "float", the
				-- theme knows to allow setting floating point numbers like 1.5.
				type= "int",
				-- The min and max fields optionally control the range of the number.
				-- The engine will force the value from the theme to be inside this
				-- range.  If min and max are not provided, the theme can set any
				-- value.
				-- Using min and max should be preferred instead of a list of choices
				-- because years of people editing themes to add speed mods have
				-- proved that a short list of choices doesn't allow everyone to pick
				-- the value they want.
				min= 0, max= 2000, translation= {
					en= {title= "Particle Distance", explanation= "A higher number makes the particles fly further."},
				},
			},
			particle_life= {
				type= "float", min= .1, max= 10, translation= {
					en= {title= "Particle Life", explanation= "Number of seconds particles live for."},
				},
			},
			particle_size= {
				type= "int", min= 0, max= 2000, translation= {
					en= {title= "Particle Size", explanation= "A higher number makes the particles larger."},
				},
			},
			num_particles= {
				type= "int", min= 0, max= 64, translation= {
					en= {title= "Particle Count", explanation= "A higher number makes the particles lag gameplay more."},
				},
			},
			particle_blend= {
				-- For string fields, a list of choices must be provided.  The engine
				-- will ensure that the value the theme sets is in the list choices.
				-- Note that the choices also have entries in the translation table.
				choices= {
					"BlendMode_Normal", "BlendMode_Add", "BlendMode_Subtract",
					"BlendMode_Modulate", "BlendMode_CopySrc", "BlendMode_AlphaMask",
					"BlendMode_AlphaKnockOut", "BlendMode_AlphaMultiply",
					"BlendMode_WeightedMultiply", "BlendMode_InvertDest",
					"BlendMode_NoEffect"},
				translation= {
					en= {
						title= "Particle Blend Mode", explanation= "Changes how the particles are blended.",
						choices= {
							"Normal", "Add", "Subtract",
							"Modulate", "Copy Source", "Alpha Mask",
							"Alpha Knock Out", "Alpha Multiply",
							"Weighted Multiply", "Invert Dest",
							"No Effect"},
					},
				},
			},
		},
		receptors= {
			translation= {
				en= {title= "Receptors", explanation= "Options for receptor effects."},
			},
			warning_time= {
				type= "float", min= 0, max= 10, translation= {
					en= {title= "Warning Time", explanation= "Turns the receptor red seconds before an arrow arrives."},
				},
			},
		},
	},
}
