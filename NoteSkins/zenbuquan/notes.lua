-- This hold length data makes the bodies look a bit screwy, but at least the
-- cap doesn't stick past the note.
local cap_len= .125
local hold_len_data= {
	start_note_offset= -cap_len, end_note_offset= cap_len,
	head_pixs= 3, body_pixs= 9, tail_pixs= 3}
local function generic_hold(anim_frames)
	return {
		state_map= NoteSkin.single_quanta_state_map(anim_frames),
		textures= {"hold_bodies"},
		flip= "TexCoordFlipMode_None",
		length_data= hold_len_data,
		disable_filtering= true,
	}
end
local column_width= 64
local function zoom_tap(self)
	self:zoom(column_width / 11):SetTextureFiltering(false)
end
return function(button_list)
	-- I was going to make this support tons of quantizations for humor, but
	-- 12ths came out the wrong color.  I'm guessing 720720 parts_per_beat
	-- goes over a precision limit or similar.
--	local tap_states= gen_state_map(4, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16})
	local tap_states= NoteSkin.generic_state_map(5, {1, 2, 3, 4, 6, 8, 12, 16, 24, 32})
	local mine_states= NoteSkin.generic_state_map(4, {1, 2, 3, 4, 6, 8, 12, 16, 24, 32})
	local columns= {}
	local holds= {
		TapNoteSubType_Hold= {
			generic_hold{1},
			generic_hold{2, 3, 4, 5},
		},
		TapNoteSubType_Roll= {
			generic_hold{6},
			generic_hold{7, 8, 9, 10},
		},
		--[[
		TapNoteSubType_Checkpoint= {
			generic_hold{11},
			generic_hold{12, 13, 14, 15},
		},
		]]
	}
	for i, button in ipairs(button_list) do
		columns[i]= {
			width= 64, padding= 0,
			anim_time= 1, anim_uses_beats= true,
			taps= {
				NoteSkinTapPart_Tap= {
					state_map= tap_states,
					actor= Def.Sprite{Texture= "tap_note", InitCommand= zoom_tap}},
				NoteSkinTapPart_Mine= {
					state_map= mine_states,
					actor= Def.Sprite{Texture= "mine_note", InitCommand= zoom_tap}},
				NoteSkinTapPart_Lift= {
					state_map= tap_states,
					actor= Def.Sprite{Texture= "lift_note", InitCommand= zoom_tap}},
			},
			holds= holds,
			reverse_holds= holds,
		}
	end
	return {columns= columns, vivid_operation= true}
end
