local skin_name= Var("skin_name")

return function(button_list, stepstype, skin_parameters)
	local rots=  {Left= 90, Down= 0, Up= 180, Right= 270, UpLeft= 135, UpRight= 225, DownLeft= 45, DownRight= 315}
	local single_states = NoteSkin.single_quanta_state_map{1}
	local inactive_states = NoteSkin.single_quanta_state_map{2}
	
	local three_d = skin_parameters.treedee
	local vivid = skin_parameters.vivid
	
	local taps, tap_states, anim_hora, quantum_hora
	local hamap, himap
	local tap_note, hold_note, roll_note
	local tex, state
	
	if three_d then
		if vivid then
			taps = "3d-vivid"
			anim_hora = 8
			quantum_hora = 2
		
		
			tap_states= {
				parts_per_beat= 48, quanta= {
					{per_beat= 1, trans_y= 0, trans_x= 0},
					{per_beat= 2, trans_y= 0, trans_x= 1/8},
					{per_beat= 3, trans_y= 0, trans_x= 2/8},
					{per_beat= 4, trans_y= 0, trans_x= 3/8},
					{per_beat= 6, trans_y= 0, trans_x= 4/8},
					{per_beat= 8, trans_y= 0, trans_x= 5/8},
					{per_beat= 12, trans_y= 0, trans_x= 6/8},
					{per_beat= 16, trans_y= 0, trans_x= 7/8},
				},
			}
		
			hamap = NoteSkin.single_quanta_state_map{2}
			himap = NoteSkin.single_quanta_state_map{3}
		
			tex = tap_states
			state = nil
		else
			taps = "3d.txt"
			anim_hora = 2
			quantum_hora = 1
		
			tap_states= {
				parts_per_beat= 48, quanta= {
					{per_beat= 1, trans_x= 0, trans_y= 0},
					{per_beat= 2, trans_x= 0, trans_y= 1/8},
					{per_beat= 3, trans_x= 0, trans_y= 2/8},
					{per_beat= 4, trans_x= 0, trans_y= 3/8},
					{per_beat= 6, trans_x= 0, trans_y= 4/8},
					{per_beat= 8, trans_x= 0, trans_y= 5/8},
					{per_beat= 12, trans_x= 0, trans_y= 6/8},
					{per_beat= 16, trans_x= 0, trans_y= 7/8},
				},
			}
		
			hamap = single_states
			himap = nil
		
			tex = tap_states
			state = nil
		end
	elseif vivid then
		taps = "vivid-taps"
		tap_states = NoteSkin.generic_state_map(32, {1})
		anim_hora = 8
		quantum_hora = 2
		
		hamap = NoteSkin.single_quanta_state_map{2}
		himap = NoteSkin.single_quanta_state_map{3}
		
		tex = nil
		state = tap_states
	else
		taps = "taps"
		tap_states =  NoteSkin.generic_state_map(8, {1, 2, 3, 4, 6, 8, 12, 16})
		anim_hora = 2
		quantum_hora = 1
		
		hamap = single_states
		himap = nil
		
		tex = nil
		state = tap_states
	end
	
	
	local hold_tex = "holds/hold bodies"
	local roll_tex = "rolls/roll bodies"
	
	local hold_data ={
		start_note_offset = 0,
		end_note_offset = 0.5,
		head_pixs = 32,
		body_pixs = 32,
		tail_pixs = 32,
	}
	
	local columns = {}
	for i, button in ipairs(button_list) do
		himap = himap or hamap
		columns[i] = {
			width = 64,
			hold_gray_percent = .125,
			anim_time = anim_hora,
			quantum_time= quantum_hora,
			anim_uses_beats = true,
			use_hold_heads_for_taps_on_row= not vivid,
			
			taps = {
				NoteSkinTapPart_Tap = {
					state_map = state,
					texture_map = tex,
					actor = LoadActor(taps)..{ InitCommand=function(self) self:rotationz(rots[button]):loop(true) end }
				},
				NoteSkinTapPart_Mine = {
					state_map = single_states,
					actor = Def.Sprite{
						Texture="mine",
						InitCommand= cmd(effectclock,"beat";spin,effectmagnitude,0,0,-180),
					},
				},
				NoteSkinTapPart_Lift = {
					state_map = single_states,
					actor = LoadActor("lift")
				},
			},
			
			optional_taps = {
				NoteSkinTapOptionalPart_HoldHead = { 
					state_map = hamap,
					inactive_state_map = himap,
					actor = LoadActor("holds/hold heads")..{ InitCommand=cmd(rotationz,rots[button]) }
				},
				NoteSkinTapOptionalPart_RollHead = {
					state_map = hamap,
					inactive_state_map = himap,
					actor = LoadActor("rolls/roll heads")..{ InitCommand=cmd(rotationz,rots[button]) }
				}
			},
			
			holds = {
				TapNoteSubType_Hold = {
					{ -- inactive
						state_map = inactive_states,
						textures = {hold_tex},
						length_data = hold_data
					},
					{ -- active
						state_map = single_states,
						textures = {hold_tex},
						length_data = hold_data
					}
				},
				TapNoteSubType_Roll = {
					{ -- inactive
						state_map = inactive_states,
						textures = {roll_tex},
						length_data = hold_data
					},
					{ -- active
						state_map = single_states,
						textures = {roll_tex},
						length_data = hold_data
					}
				},
				
			},
			
			reverse_holds = {
				TapNoteSubType_Hold = {
					{ -- inactive
						state_map = inactive_states,
						textures = {hold_tex},
						length_data = hold_data
					},
					{ -- active
						state_map = single_states,
						textures = {hold_tex},
						length_data = hold_data
					}
				},
				TapNoteSubType_Roll = {
					{ -- inactive
						state_map = inactive_states,
						textures = {roll_tex},
						length_data = hold_data
					},
					{ -- active
						state_map = single_states,
						textures = {roll_tex},
						length_data = hold_data
					}
				},	
				
			}
			
						
		}
	end
	return {columns=columns, vivid_operation=true}
end
