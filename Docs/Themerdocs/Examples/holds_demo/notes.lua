return function(button_list, stepstype, skin_parameters)
	local parts_per_beat= 48
	local tap_state_map= {
		parts_per_beat= parts_per_beat, quanta= {
			{per_beat= 1, states= {1}}, -- 4th
			{per_beat= 2, states= {3}}, -- 8th
			{per_beat= 3, states= {5}}, -- 12th
			{per_beat= 4, states= {7}}, -- 16th
			{per_beat= 6, states= {9}}, -- 24th
			{per_beat= 8, states= {11}}, -- 32nd
			{per_beat= 12, states= {13}}, -- 48th
			{per_beat= 16, states= {15}}, -- 64th
		},
	}
	-- Taps use the even states, lifts use the odd states.
	local lift_state_map= DeepCopy(tap_state_map)
	for i, quanta in ipairs(lift_state_map.quanta) do
		quanta.states[1]= quanta.states[1] + 1
	end
	local mine_state_map= tap_state_map
	local hold_active_map= {}
	local hold_inactive_map= {}
	if skin_parameters.hold_style == "minimal_quantized" then
		hold_inactive_map= lift_state_map
		hold_active_map= tap_state_map
	else
		hold_active_map= NoteSkin.single_quanta_state_map{2}
		hold_inactive_map= NoteSkin.single_quanta_state_map{1}
	end
	local hold_length= {
		tilted_full_capped= {
			pixels_before_note= 32,
			topcap_pixels= 64,
			body_pixels= 64,
			bottomcap_pixels= 64,
			pixels_after_note= 32,
		},
		tilted_no_bottomcap= {
			pixels_before_note= 32,
			topcap_pixels= 64,
			body_pixels= 64,
			bottomcap_pixels= 0,
			pixels_after_note= 0,
		},
		tilted_no_topcap= {
			pixels_before_note= 0,
			topcap_pixels= 0,
			body_pixels= 64,
			bottomcap_pixels= 64,
			pixels_after_note= 32,
		},
		tilted_no_caps= {
			pixels_before_note= 0,
			topcap_pixels= 0,
			body_pixels= 64,
			bottomcap_pixels= 0,
			pixels_after_note= 0,
		},
		flat_full_capped= {
			pixels_before_note= 32,
			topcap_pixels= 64,
			body_pixels= 64,
			bottomcap_pixels= 64,
			pixels_after_note= 32,
		},
		flat_no_bottomcap= {
			pixels_before_note= 32,
			topcap_pixels= 64,
			body_pixels= 64,
			bottomcap_pixels= 0,
			pixels_after_note= 0,
		},
		flat_no_topcap= {
			pixels_before_note= 0,
			topcap_pixels= 0,
			body_pixels= 64,
			bottomcap_pixels= 64,
			pixels_after_note= 32,
		},
		flat_no_caps= {
			pixels_before_note= 0,
			topcap_pixels= 0,
			body_pixels= 64,
			bottomcap_pixels= 0,
			pixels_after_note= 0,
		},
		minimal= {
			pixels_before_note= 0,
			topcap_pixels= 0,
			body_pixels= 16,
			bottomcap_pixels= 0,
			pixels_after_note= 0,
			needs_jumpback= false,
		},
		minimal_quantized= {
			-- Sprite sheets require a margin around each frame, which would show
			-- up as holes in the body if body_pixels was 32 and the others were 0.
			pixels_before_note= 2,
			topcap_pixels= 2,
			body_pixels= 28,
			bottomcap_pixels= 2,
			pixels_after_note= 2,
			needs_jumpback= true,
		},
	}
	local tap_texture= skin_parameters.note_style .. "_tap"
	local hold_texture= skin_parameters.hold_style .. "_hold"
	local columns= {}
	for i, button in ipairs(button_list) do
		columns[i]= {
			hold_gray_percent= .25,
			taps= {
				NoteSkinTapPart_Tap= {
					state_map= tap_state_map,
					actor= Def.Sprite{Texture= tap_texture},
				},
				NoteSkinTapPart_Mine= {
					state_map= tap_state_map,
					-- I wonder if adding a black glow makes it different enough.
					actor= Def.Sprite{Texture= tap_texture,
						InitCommand= function(self) self:glow{0, 0, 0, 1} end,
					},
				},
				NoteSkinTapPart_Lift= {
					state_map= lift_state_map,
					actor= Def.Sprite{Texture= tap_texture},
				},
			},
			holds= {
				TapNoteSubType_Hold= {
					{
						state_map= hold_inactive_map,
						textures= {hold_texture},
						length_data= hold_length[skin_parameters.hold_style],
					},
					{
						state_map= hold_active_map,
						textures= {hold_texture},
						length_data= hold_length[skin_parameters.hold_style],
					},
				},
				TapNoteSubType_Roll= {
					{
						state_map= hold_inactive_map,
						textures= {hold_texture},
						length_data= hold_length[skin_parameters.hold_style],
					},
					{
						state_map= hold_active_map,
						textures= {hold_texture},
						length_data= hold_length[skin_parameters.hold_style],
					},
				},
			},
		}
		columns[i].reverse_holds= columns[i].holds
	end
	return {columns= columns}
end
