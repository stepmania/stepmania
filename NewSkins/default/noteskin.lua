return {
	-- notes is the file to use to load the note columns, the taps and holds.
	notes= "notes.lua",
	-- layers_below_notes is a list of files to load things in the columns that
	-- are displayed underneath the notes.  Typically, this is just the
	-- receptors.  The layers are rendered in the order they are in the table,
	-- so the first layer is on the bottom.
	-- receptors.lua has the explanation of the requirements of a layer file.
	layers_below_notes= {"receptors.lua"},
	-- layers_above_notes is the same as layers_below_notes, but its contents
	-- are rendered after the notes, so they appear on top of the notes.
	layers_above_notes= {"explosions.lua"},
	-- Since all layers are considered the same, messages such as
	-- judgment and step actions are sent to all layers.  This means you can
	-- make receptors that respond to judgments, or explosions that respond to
	-- steps, or whatever.
	supports_all_buttons= false,
	buttons= {"Left", "Down", "Up", "Right"},
	-- The fallback entry is optional.  It can be used to name another noteskin
	-- to fall back on if a hold texture or one of the files listed above is
	-- not found.
	fallback= "",
	skin_parameters= {
		explosions= {
			particles= true,
			particle_dist= 512,
			particle_life= 1,
			particle_size= 32,
			num_particles= 16,
			particle_blend= "BlendMode_WeightedMultiply",
		},
		receptors= {
			warning_time= 2,
		},
	},
	skin_parameter_info= {
		explosions= {
			translation= {
				en= {title= "Explosions", explanation= "Options for the explosion effects."},
			},
			particles= {
				translation= {
					en= {title= "Particles", explanation= "When set to true, particles will fly out when a note is hit."},
			}},
			particle_dist= {
				type= "int", min= 0, max= 2000, translation= {
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
				type= "int", min= 0, max= 2000, translation= {
					en= {title= "Particle Count", explanation= "A higher number makes the particles lag gameplay more."},
				},
			},
			particle_blend= {
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
				type= "int", min= 0, max= 10, translation= {
					en= {title= "Warning Time", explanation= "Turns the receptor red seconds before an arrow arrives."},
				},
			},
		},
	},
}
