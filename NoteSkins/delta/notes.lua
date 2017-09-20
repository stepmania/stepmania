local skin_name = Var "skin_name"
return function(button_list, stepstype)
	local rots = {
		Left      = 90,
		Down      = 0,
		Up        = 180,
		Right     = 270,
		UpLeft    = 135,
		UpRight   = 225, 
		DownLeft  = 45,
		DownRight = 315
	}
	local hold_flips = {
		Left  = "TexCoordFlipMode_None",
		Right = "TexCoordFlipMode_None",
		Down  = "TexCoordFlipMode_None",
		Up    = "TexCoordFlipMode_None",
	}
	local roll_flips     = hold_flips
	local rev_hold_flips = hold_flips
	local rev_roll_flips = hold_flips

	local tap_width  = {}
	local tap_redir  = {}
	local hold_redir = {}

	for _, button in pairs(button_list) do
		tap_redir[button]  = "Down"
		hold_redir[button] = "Down"
		tap_width[button]  = 64
	end

	local parts_per_beat = 48
	local tap_state_map = {
		parts_per_beat = parts_per_beat,
		quanta = {}
	}

	local snaps_per_beat = { 1, 2, 3, 4, 6, 8, 12, 16 }
	for i=0,7 do
		local quant = {
			per_beat = snaps_per_beat[i+1],
			states = {}
		}
		for j=1,12 do
			quant.states[j] = i*12+j
		end
		table.insert(tap_state_map.quanta, quant)
	end
	
	local lift_state_map = {
		parts_per_beat = 1,
		quanta = {
			{ per_beat = 1, states = tap_state_map.quanta[#tap_state_map.quanta].states }
		}
	}

	local hold_length = {
		start_note_offset = -0.5,
		end_note_offset   = 0.5,
		head_pixs         = 32,
		body_pixs         = 256,
		tail_pixs         = 64
	}

	local mine_state_map = {
		parts_per_beat = 1,
		quanta = {
			{ per_beat = 1, states = { 1, 2, 3, 4, 5, 6, 7, 8 } }
		}
	}
	local active_state_map = {
		parts_per_beat = parts_per_beat,
		quanta = {
			{ per_beat = 1, states = { 1 } },
		},
	}
	local inactive_state_map = {
		parts_per_beat = parts_per_beat,
		quanta = {
			{ per_beat = 1, states = { 2 } },
		},
	}
	local columns = {}
	for i, button in ipairs(button_list) do
		local hold_tex = "Down Hold 2x1.png"
		local roll_tex = "Down Roll 2x1.png"
		columns[i] = {
			width           = tap_width[button],
			anim_time       = 2,
			anim_uses_beats = true,
			padding         = 0,
			taps            = {
				NoteSkinTapPart_Tap = {
					state_map    = tap_state_map,
					actor        = Def.Sprite {
						Texture = tap_redir[button].." Tap Note 12x8.png",
						InitCommand  = function(self)
							self:rotationz(rots[button])
						end
					}
				},
				NoteSkinTapPart_Mine = {
					state_map = mine_state_map,
					actor     = Def.Sprite {
						Texture = "Mine 4x2.png"
					}
				},
				NoteSkinTapPart_Lift = {
					state_map = lift_state_map,
					actor     = Def.Sprite{
						Texture = tap_redir[button] .. " Tap Note 12x8.png",
						InitCommand = function(self)
							self:rotationz(rots[button])
							self:zoom(0.75)
						end
					}
				},
			},
			holds = {
				TapNoteSubType_Hold = {
					{
						state_map   = inactive_state_map,
						textures    = { hold_tex },
						flip        = hold_flips[button],
						length_data = hold_length,
					},
					{
						state_map   = active_state_map,
						textures    = { hold_tex },
						flip        = hold_flips[button],
						length_data = hold_length,
					},
				},
				TapNoteSubType_Roll = {
					{
						state_map   = inactive_state_map,
						textures    = { roll_tex },
						flip        = roll_flips[button],
						length_data = hold_length,
					},
					{
						state_map   = active_state_map,
						textures    = { roll_tex },
						flip        = roll_flips[button],
						length_data = hold_length,
					},
				},
			},
			reverse_holds = {
				TapNoteSubType_Hold = {
					{
						state_map   = inactive_state_map,
						textures    = { hold_tex },
						flip        = rev_hold_flips[button],
						length_data = hold_length,
					},
					{
						state_map   = active_state_map,
						textures    = { hold_tex },
						flip        = rev_hold_flips[button],
						length_data = hold_length,
					},
				},
				TapNoteSubType_Roll = {
					{
						state_map   = inactive_state_map,
						textures    = { roll_tex },
						flip        = rev_roll_flips[button],
						length_data = hold_length,
					},
					{
						state_map   = active_state_map,
						textures    = { roll_tex },
						flip        = rev_roll_flips[button],
						length_data = hold_length,
					},
				},
			},
		}
	end
	return {
		columns = columns,
		vivid_operation = false,
	}
end
