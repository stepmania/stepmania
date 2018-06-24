local skin_name= Var("skin_name")
return function(button_list, stepstype)
	local rots= {
		Left= 90, Down= 0, Up= 180, Right= 270,
		UpLeft= 90, UpRight= 180, 
		DownLeft= 0, DownRight= 270
	}
	local hold_flips= {
		Left= "TexCoordFlipMode_None", Right= "TexCoordFlipMode_None",
		Down= "TexCoordFlipMode_None", Up= "TexCoordFlipMode_None",
	}
	local roll_flips= {
		Left= "TexCoordFlipMode_None", Right= "TexCoordFlipMode_None",
		Down= "TexCoordFlipMode_None", Up= "TexCoordFlipMode_None",
	}
	local rev_hold_flips= {
		Left= "TexCoordFlipMode_None", Right= "TexCoordFlipMode_None",
		Down= "TexCoordFlipMode_None", Up= "TexCoordFlipMode_None",
	}
	local rev_roll_flips= {
		Left= "TexCoordFlipMode_None", Right= "TexCoordFlipMode_None",
		Down= "TexCoordFlipMode_None", Up= "TexCoordFlipMode_None",
	}
	local tap_redir= {
		Left= "down", Right= "down", Down= "down", Up= "down", 
		UpLeft= "DownLeft", UpRight= "DownLeft", -- shared for dance and pump
		DownLeft= "DownLeft", DownRight= "DownLeft",
	}
	local tap_width= {
		Left= 64, Down= 64, Up= 64, Right= 64,
		UpLeft= 48, UpRight= 48, 
		DownLeft= 48, DownRight= 48,
	}
	local hold_redir= {
		Left= "down", Right= "down", Down= "down", Up= "down", 
		UpLeft= "down", UpRight= "down", -- shared for dance and pump
	}
	
	local parts_per_beat = 48
	local tap_state_map= NoteSkin.generic_state_map(4, {1, 2, 3, 4})
	
	local lift_state_map= {
		parts_per_beat= 1, quanta= {{per_beat= 1, states= {1}}}
	}
		
	local hold_length= {
		pixels_before_note= 32,
		topcap_pixels= 32,
		body_pixels= 64,
		bottomcap_pixels= 32,
		pixels_after_note= 32,
		needs_jumpback= false,
	}
	
	local head_state_map_active = NoteSkin.single_quanta_state_map{1}
	local head_state_map_inactive = NoteSkin.single_quanta_state_map{2}
	
	-- Mines only have a single frame in the graphics.
	local mine_state_map= {
		parts_per_beat= 1, quanta= {{per_beat= 1, states= {1}}}}
	local active_state_map= {
		parts_per_beat= parts_per_beat, quanta= {
			{per_beat= 1, states= {1}},
		},
	}
	local inactive_state_map= {
		parts_per_beat= parts_per_beat, quanta= {
			{per_beat= 1, states= {2}},
		},
	}
	local columns= {}
	for i, button in ipairs(button_list) do
		local hold_tex= hold_redir[button].." hold 2x1 (doubleres).png"
		local roll_tex= hold_redir[button].." roll 2x1 (doubleres).png"
		columns[i]= {
			width= tap_width[button],
			anim_time= 1,
			anim_uses_beats= true,
			padding= 0,
			taps= {
				NoteSkinTapPart_Tap= {
					state_map= tap_state_map,
					actor= Def.ActorFrame {
						Def.Sprite{Texture= tap_redir[button].." Tap Note 4x4.png",
						InitCommand= function(self) self:rotationz(rots[button]) end},
					}
				},
				NoteSkinTapPart_Mine= {
					state_map= mine_state_map,
					actor= Def.Sprite{Texture= "mine.png",
					InitCommand= function(self) self:spin():effectclock('beat'):effectmagnitude(0,0,-33) end}},
				NoteSkinTapPart_Lift= { 
					state_map= lift_state_map,
					actor= Def.Sprite{Texture= tap_redir[button].." Lift.png"}},
			},

			optional_taps= {
				NoteSkinTapOptionalPart_HoldHead= {
					state_map= head_state_map_active,
					inactive_state_map= head_state_map_inactive,
						actor= Def.ActorFrame {
							Def.Sprite{Texture= tap_redir[button].." Hold Head 2x1.png",
							InitCommand= function(self) self:rotationz(rots[button]) end},
						}				
					},
				NoteSkinTapOptionalPart_RollHead= {
					state_map= head_state_map_active,
					inactive_state_map= head_state_map_inactive,
						actor= Def.ActorFrame {
							Def.Sprite{Texture= tap_redir[button].." Roll Head 2x1.png",
							InitCommand= function(self) self:rotationz(rots[button]) end},
						}				
					},
				},
			holds= {
				TapNoteSubType_Hold= {
					{
						state_map= inactive_state_map,
						textures= {hold_tex},
						flip= hold_flips[button],
						length_data= hold_length,
					},
					{
						state_map= active_state_map,
						textures= {hold_tex},
						flip= hold_flips[button],
						length_data= hold_length,
					},
				},
				TapNoteSubType_Roll= {
					{
						state_map= inactive_state_map,
						textures= {roll_tex},
						flip= roll_flips[button],
						length_data= hold_length,
					},
					{
						state_map= active_state_map,
						textures= {roll_tex},
						flip= roll_flips[button],
						length_data= hold_length,
					},
				},
			},
			reverse_holds= {
				TapNoteSubType_Hold= {
					{
						state_map= inactive_state_map,
						textures= {hold_tex},
						flip= rev_hold_flips[button],
						length_data= hold_length,
					},
					{
						state_map= active_state_map,
						textures= {hold_tex},
						flip= rev_hold_flips[button],
						length_data= hold_length,
					},
				},
				TapNoteSubType_Roll= {
					{
						state_map= inactive_state_map,
						textures= {roll_tex},
						flip= rev_roll_flips[button],
						length_data= hold_length,
					},
					{
						state_map= active_state_map,
						textures= {roll_tex},
						flip= rev_roll_flips[button],
						length_data= hold_length,
					},
				},
			},
		}
	end
	return {
		columns= columns,
		vivid_operation= true, -- output 200%
	}
end
