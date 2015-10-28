-- See explanation comments in default/notes.lua.
-- Comments in here only address multiplayer or routine mode.
local skin_name= Var("skin_name")
return function(button_list, stepstype)
	-- The stepstype_is_multiplayer function returns true if the stepstype puts
	-- multiple players on one notefield.
	-- Calling the GAMEMAN function should be more future proof than noteskins
	-- having lists of which stepstypes are multiplayer.
	-- This terminology is used instead of calling it routine mode because I
	-- like calling it multiplayer more.
	if GAMEMAN:stepstype_is_multiplayer(stepstype) then
		Trace("Loading noteskin for multiplayer.")
	end
	local rots= {Left= 90, Down= 0, Up= 180, Right= 270}
	local hold_flips= {
		Left= "TexCoordFlipMode_None", Right= "TexCoordFlipMode_X",
		Down= "TexCoordFlipMode_None", Up= "TexCoordFlipMode_Y",
	}
	local roll_flips= {
		Left= "TexCoordFlipMode_None", Right= "TexCoordFlipMode_X",
		Down= "TexCoordFlipMode_None", Up= "TexCoordFlipMode_None",
	}
	local rev_hold_flips= {
		Left= "TexCoordFlipMode_None", Right= "TexCoordFlipMode_X",
		Down= "TexCoordFlipMode_Y", Up= "TexCoordFlipMode_None",
	}
	local rev_roll_flips= {
		Left= "TexCoordFlipMode_None", Right= "TexCoordFlipMode_X",
		Down= "TexCoordFlipMode_Y", Up= "TexCoordFlipMode_Y",
	}
	local hold_buttons= {
		Left= "left", Right= "left", Down= "down", Up= "down",
	}
	local roll_buttons= {
		Left= "left", Right= "left", Down= "down", Up= "up",
	}
	local parts_per_beat= 48
	local tap_state_map= {
		parts_per_beat= parts_per_beat, quanta= {
			{per_beat= 1, states= {1, 2}}, -- 4th
			{per_beat= 2, states= {3, 4}}, -- 8th
			{per_beat= 3, states= {5, 6}}, -- 12th
			{per_beat= 4, states= {7, 8}}, -- 16th
			{per_beat= 6, states= {9, 10}}, -- 24th
			{per_beat= 8, states= {11, 12}}, -- 32nd
			{per_beat= 12, states= {13, 14}}, -- 48th
			{per_beat= 16, states= {15, 16}}, -- 64th
		},
	}
	local mine_state_map= {
		parts_per_beat= 1, quanta= {{per_beat= 1, states= {1}}}}
	local active_state_map= {
		parts_per_beat= parts_per_beat, quanta= {
			{per_beat= 1, states= {1, 2}}, -- 4th
			{per_beat= 2, states= {5, 6}}, -- 8th
			{per_beat= 3, states= {9, 10}}, -- 12th
			{per_beat= 4, states= {13, 14}}, -- 16th
			{per_beat= 6, states= {17, 18}}, -- 24th
			{per_beat= 8, states= {21, 22}}, -- 32nd
			{per_beat= 12, states= {25, 26}}, -- 48th
			{per_beat= 16, states= {29, 30}}, -- 64th
		},
	}
	local inactive_quanta= {}
	for i, quantum in ipairs(active_state_map.quanta) do
		local states= {}
		for s, state in ipairs(quantum.states) do
			states[s]= state + 2
		end
		inactive_quanta[i]= {per_beat= quantum.per_beat, states= states}
	end
	local inactive_state_map= {parts_per_beat= parts_per_beat, quanta= inactive_quanta}
	local columns= {}
	-- There are two ways to support multiplayer mode:
	-- 1. Mask mode.  The notefield applies a mask to taps and holds.  The 
	--    alpha channel in the mask sets where the color is applied.
	-- 2. Quanta mode.  The notefield uses a different quanta for each player.
	--    So the first quanta in the state map is for player 1, the second for
	--    player 2, and so on.
	-- The theme (or player) chooses which mode is used.  If the noteskin does
	-- not supply masks for the holds, then the field will always use Quanta
	-- mode.
	--
	-- If a noteskin wants to have holds that are playerized in multiplayer
	-- mode, but not quantized in single player mode, then it should use the
	-- GAMEMAN:stepstype_is_multiplayer function to detect the mode and use a
	-- different state map for holds.  For example:
	--
	-- local hold_state_map= {}
	-- if GAMEMAN:stepstype_is_multiplayer(stepstype) then
	-- 	hold_state_map= {parts_per_beat= parts_per_beat, quanta= {
	-- 		{per_beat= 1, states= {1, 2}}}}
	-- else
	-- 	hold_state_map= {parts_per_beat= parts_per_beat, quanta= {
	-- 		{per_beat= 1, states= {1, 2}},
	-- 		{per_beat= 1, states= {5, 6}},
	-- 		{per_beat= 1, states= {9, 10}},
	-- 		{per_beat= 1, states= {13, 14}},
	-- 	}}
	-- end
	--
	-- If a noteskin wants to support masks for taps but not have masks on
	-- holds, then it should use a hold mask that is transparent everywhere.
	--
	-- If the mask texture is not the same size as the texture for the thing
	-- being masked (hold or tap), the mask texture will be stretched and look
	-- wrong.
	for i, button in ipairs(button_list) do
		-- NEWSKIN:get_path is used to fetch a path to a file in this noteskin
		-- or a noteskin that it falls back on.  This routine noteskin falls
		-- back on the default noteskin, so NEWSKIN:get_path is used to get the
		-- paths to the sprite textures.
		-- NEWSKIN:get_path is automatically applied to hold and roll textures,
		-- so using it when setting hold and roll textures is harmlessly
		-- redundant.
		local hold_tex= NEWSKIN:get_path(skin_name, hold_buttons[button] .. "_hold 8x4.png")
		local roll_tex= roll_buttons[button] .. "_roll 8x4.png"
		-- Normally, this noteskin should have different masks for left and down
		-- because the holds are different, but I didn't feel like making two
		-- masks.
		local hold_mask_tex= "down_mask_hold 8x4.png"
		columns[i]= {
			width= 64,
			padding= 0,
			taps= {
				NewSkinTapPart_Tap= {
					state_map= tap_state_map,
					actor= Def.Sprite{
						Texture= NEWSKIN:get_path(skin_name, "tap_note 2x8.png"),
						-- The Mask field sets the mask texture used for the tap.
						Mask= NEWSKIN:get_path(skin_name, "tap_mask 2x8.png")
				}},
				NewSkinTapPart_Mine= {
					state_map= mine_state_map,
					actor= Def.Sprite{Texture= NEWSKIN:get_path(skin_name, "mine.png")}},
				NewSkinTapPart_Lift= { -- fuck lifts
					state_map= mine_state_map,
					actor= Def.Sprite{Texture= NEWSKIN:get_path(skin_name, "mine.png")}},
			},
			holds= {
				TapNoteSubType_Hold= {
					{
						state_map= inactive_state_map,
						textures= {hold_tex},
						flip= hold_flips[button],
					},
					{
						state_map= active_state_map,
						textures= {hold_tex},
						flip= hold_flips[button],
					},
				},
				TapNoteSubType_Roll= {
					{
						state_map= inactive_state_map,
						textures= {roll_tex},
						flip= roll_flips[button],
					},
					{
						state_map= active_state_map,
						textures= {roll_tex},
						flip= roll_flips[button],
					},
				},
			},
			reverse_holds= {
				TapNoteSubType_Hold= {
					{
						state_map= inactive_state_map,
						textures= {hold_tex},
						flip= rev_hold_flips[button],
					},
					{
						state_map= active_state_map,
						textures= {hold_tex},
						flip= rev_hold_flips[button],
					},
				},
				TapNoteSubType_Roll= {
					{
						state_map= inactive_state_map,
						textures= {roll_tex},
						flip= rev_roll_flips[button],
					},
					{
						state_map= active_state_map,
						textures= {roll_tex},
						flip= rev_roll_flips[button],
					},
				},
			},
			-- hold_masks and hold_reverse_masks are for the mask textures used for
			-- each hold type.
			hold_masks= {
				TapNoteSubType_Hold= hold_mask_tex,
				TapNoteSubType_Roll= hold_mask_tex,
			},
			hold_reverse_masks= {
				TapNoteSubType_Hold= hold_mask_tex,
				TapNoteSubType_Roll= hold_mask_tex,
			},
			rotations= {
				NewSkinTapPart_Tap= rots[button],
				NewSkinTapPart_Mine= 0,
				NewSkinTapPart_Lift= rots[button],
			},
		}
	end
	return {
		columns= columns,
		vivid_operation= true, -- output 200%
	}
end
