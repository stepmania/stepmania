return function(button_list)
	local rots= {ParaLeft= 90, ParaUpLeft= 135, ParaUp= 180, ParaUpRight= 225, ParaRight= 270}
	local tap_states= NoteSkin.single_quanta_state_map{1, 2, 3, 4, 5, 6, 7, 8}
	local mine_state_map= NoteSkin.single_quanta_state_map{1, 2, 3, 4, 5, 6, 7, 8}
	local hold_head_active_map= NoteSkin.single_quanta_state_map({1})
	local hold_head_inactive_map= NoteSkin.single_quanta_state_map({2})
	local hold_active_map= NoteSkin.single_quanta_state_map({1})
	local hold_inactive_map= NoteSkin.single_quanta_state_map({2})
	local roll_active_map= NoteSkin.single_quanta_state_map({3})
	local roll_inactive_map= NoteSkin.single_quanta_state_map({4})
	local hold_length= {
		start_note_offset= -.125, end_note_offset= .125,
		head_pixs= 16, body_pixs= 16, tail_pixs= 16,
	}
	local function a_hold(states)
		return {
			state_map= states,
			textures= {"holds_and_rolls"},
			length_data= hold_length,
		}
	end
	local holds= {
		TapNoteSubType_Hold= {
			a_hold(hold_inactive_map),
			a_hold(hold_active_map),
		},
		TapNoteSubType_Roll= {
			a_hold(roll_inactive_map),
			a_hold(roll_active_map),
		},
	}
	local columns= {}
	for i, button in ipairs(button_list) do
		columns[i]= {
			width= 64, padding= 0,
			taps= {
				NoteSkinTapPart_Tap= {
					state_map= tap_states,
					actor= Def.Sprite{Texture= "_down tap note",
						InitCommand= function(self) self:rotationz(rots[button]) end}},
				NoteSkinTapPart_Mine= {
					state_map= mine_state_map,
					actor= Def.Sprite{Texture= "mine"}},
				NoteSkinTapPart_Lift= {
					state_map= tap_states,
					actor= Def.Sprite{Texture= "_down tap note",
						InitCommand= function(self) self:rotationz(rots[button])
							:blend("BlendMode_InvertDest") end}},
			},
			optional_taps= {
				NoteSkinTapOptionalPart_HoldHead= {
					state_map= hold_head_active_map,
					inactive_state_map= hold_head_inactive_map,
					actor= Def.Sprite{Texture= "hold_head",
						InitCommand= function(self) self:rotationz(rots[button]) end}},
				NoteSkinTapOptionalPart_RollHead= {
					state_map= hold_head_active_map,
					inactive_state_map= hold_head_inactive_map,
					actor= Def.Sprite{Texture= "hold_head",
						InitCommand= function(self) self:rotationz(rots[button]) end}},
			},
			holds= holds,
			reverse_holds= holds,
		}
	end
	return {columns= columns, vivid_operation= true}
end
