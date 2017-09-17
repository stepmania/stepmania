return {
	notes= "notes.lua",
	layers= {"receptors.lua", "explosions.lua"},
	supports_all_buttons= true,
	skin_parameters= {
		note_style= "tilted",
		hold_style= "tilted_full_capped",
	},
	skin_parameter_info= {
		note_style= {
			choices= {
				"flat", "tilted",
			},
			translation= {
				en= {
					title= "Note Style",
					choices= {
						"Flat", "Tilted",
					},
				},
			},
		},
		hold_style= {
			choices= {
				"tilted_full_capped", "tilted_no_bottomcap", "tilted_no_topcap",
				"tilted_no_caps",
				"flat_full_capped", "flat_no_bottomcap", "flat_no_topcap",
				"flat_no_caps",
				"minimal", "minimal_quantized"
			},
			translation= {
				en= {
					title= "Hold Style",
					choices= {
						"Tilted Full capped", "Tilted No Bottomcap", "Tilted No Topcap",
						"Tilted No caps",
						"flat Full capped", "flat No Bottomcap", "flat No Topcap",
						"flat No caps",
						"Minimal", "Minimal Quantized"
					},
				},
			},
		},
	},
}
