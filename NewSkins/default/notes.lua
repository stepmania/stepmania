local skin_name= Var("skin_name")
-- The noteskin file returns a function that will be called to load the
-- actors for the noteskin.  The function will be passed a list of buttons.
-- Each element in button_list is the name of the button that will be used
-- for that column.
-- A button name may be repeated, for instance when the notefield is for
-- doubles, which has Left, Down, Up, Right, Left, Down, Up, Right.
-- The function must return a table containing all the information and actors
-- needed for the columns given.  Missing information or actors will be
-- filled with zeros or blank actors.
return function(button_list, stepstype)
	-- This example doesn't use the stepstype arg, but it is provided in case
	-- someone needs it.

	-- rots is a convenience conversion table to easily take care of buttons
	-- that should be rotated.
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
	-- A state_map tells Stepmania what frames to use for the quantization of
	-- a note and the current beat.
	-- First, Stepmania goes through the list of quanta in the state map to
	-- find the one that fits how far the note is from directly on the beat.
	-- Stepmania uses the per_beat field in the quantum to pick which one to
	-- use.
	-- Secondly, Stepmania finds the state in the quantum for the current beat.
	-- The vivid flag is used here to allow vivid noteskins to make different
	-- quantizations start at different points in the animation.
	-- In code, it uses this formula:
	--   if vivid then
	--     state_index= floor((quantization + beat) * #entry)
	--   else
	--     state_index= floor(beat * #entry)
	--   end
	-- The index is then wrapped so that quantization 0.75 + beat 0.75
	-- doesn't go off the end of the entry.
	-- Long explanation:
	--   Each quantization has a set of frames that it cycles through.  The
	--   current beat is used to pick the frame.  For vivid noteskins, the
	--   quantization is added to the beat to pick the frame.  If
	--   quantization+beat is greater than 1, it is shifted to put it between
	--   0 and 1.
	--   The frames are spaced equally through the beat.
	--   If there are two frames, the first frame is used while the beat is
	--   greater than or equal to 0 and less than 0.5.  The second frame is
	--   used from 0.5 to 1.
	-- This example has 8 quantizations.
	-- tap_state_map is for convenience making the noteskin.  Each
	-- NewSkinPart in each column is allowed to have its own state map, to allow
	-- them to animate differently, but that is usually not desired.
	-- Each quantization occurs a given number of times per beat.
	-- They must be arranged in ascending order of times per beat for the
	-- correct quantization to be assigned.
	-- The states element is a list of frames to use for animating the tap when
	-- it has the given quantization.
	-- If the per_beat field of a quantum is not a factor of parts_per_beat, it
	-- will not be used.
	-- The last entry in quanta will be used for parts that don't fit a known
	-- quantization.
	-- parts_per_beat must be the lowest common multiple of the per_beat fields
	-- of the quanta.  If parts_per_beat is not the lowest common multiple, the
	-- quanta that it is not a multiple of will be ignored.
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
	-- A noteskin that uses 3D notes (Def.Model) should use a texture map
	-- instead of a state map to control how the notes are quantized.
	-- A texture map is very similar to a state map.  Each quanta in a texture
	-- map has trans_x and trans_y fields for translating the texture instead
	-- of a list of states.
	-- local tap_texture_map= {
	--   parts_per_beat= parts_per_beat, quanta= {
	--     {per_beat= 1, trans_x= 0, trans_y= 0},
	--     {per_beat= 2, trans_x= .125, trans_y= 0},
	--     {per_beat= 3, trans_x= .25, trans_y= 0},
	--     {per_beat= 4, trans_x= .375, trans_y= 0},
	--     {per_beat= 6, trans_x= .5, trans_y= 0},
	--     {per_beat= 8, trans_x= .625, trans_y= 0},
	--     {per_beat= 12, trans_x= .75, trans_y= 0},
	--     {per_beat= 16, trans_x= .875, trans_y= 0},
	--   },
	-- }
	-- Taps use the even states, lifts use the odd states.
	local lift_state_map= DeepCopy(tap_state_map)
	for i, quanta in ipairs(lift_state_map.quanta) do
		quanta.states[1]= quanta.states[1] + 1
	end
	-- Mines only have a single frame in the graphics.
	local mine_state_map= {
		parts_per_beat= 1, quanta= {{per_beat= 1, states= {1}}}}
	-- Holds have active and inactive states, so they need a different state
	-- map.
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
	-- To make creating the inactive state map easier, the inactive states are
	-- assumed to be the two states after the active states.
	for i, quantum in ipairs(active_state_map.quanta) do
		local states= {}
		for s, state in ipairs(quantum.states) do
			states[s]= state + 2
		end
		inactive_quanta[i]= {per_beat= quantum.per_beat, states= states}
	end
	local inactive_state_map= {parts_per_beat= parts_per_beat, quanta= inactive_quanta}
	-- Taps are handled by a quantized_tap structure.  A quantized_tap contains
	-- an actor for the tap, a state map to use to quantize it, and a vivid
	-- memory of its world that was destroyed.
	-- If the vivid flag is not set, it will be treated as false.
	-- If you are making a vivid noteskin, you probably want to use the global
	-- vivid_operation flag that is explained later.
	-- Each element in taps is the set of quantized_taps to use for that column.
	-- There is one quantized_tap for each noteskin part.

	-- Holds are not actors at all.  Instead, they are handled through a series
	-- of tubes, like a big truck.
	-- A quantized_hold has a state map (same as a tap's state map), a set of
	-- textures for its layers, and its own vivid love for the world it
	-- protects.
	-- A hold texture must have the top cap, body, and bottom cap in each
	-- frame.  The top cap must be 1/6 of the frame height, the body must be
	-- 2/3 of the frame height, and the bottom cap must be the remaining 1/6 of
	-- the frame.  The second half of the body must be identical to the first.
	-- This is so that during rendering, the first half of the body section can
	-- be repeated to cover the length of the hold.  Putting the top cap and
	-- bottom cap in the same frame as the body avoids gap and seam problems.

	-- Each column has a set of quantized_taps for each noteskin part, a
	-- quantized_hold for the inactive and inactive states of each hold subtype
	-- (holds and rolls), and a rotations table that controls how the parts are
	-- rotated.
	local columns= {}
	for i, button in ipairs(button_list) do
		local hold_tex= hold_buttons[button] .. "_hold 8x4.png"
		local roll_tex= roll_buttons[button] .. "_roll 8x4.png"
		-- There will be a section about mask textures and routine mode at the
		-- end of the file.  Ignore mask textures and don't include them in your
		-- noteskin if there's already too much to understand.
		local hold_mask_tex= "down_mask_hold 8x4.png"
		columns[i]= {
			-- The width parameter specifies how wide the column is in pixels.
			-- Different columns are allowed to be different widths.
			-- Holds in this column will be stretched to this width.
			-- If the width is not provided by the noteskin, the default of 64 will
			-- be used.
			width= 64,
			-- The padding is the distance between this column and the adjacent
			-- column.  A padding value of 2 means 1 pixel on each side.
			-- The default is 0.
			padding= 0,
			-- anim_time is used to specify how long the animation of the taps
			-- lasts.  anim_time is optional, if it isn't set, the engine will set
			-- it to 1.
			anim_time= 1,
			-- quantum_time is how many beats the quantizations cover.
			-- quantum_time is optional, if it isn't set, the engine will default
			-- it to 1.
			quantum_time= 1,
			-- anim_uses_beats specifies whether anim_time is a number of beats or
			-- a number of seconds.
			anim_uses_beats= true,
			-- anim_time of 1 and anim_uses_beats set to true means that taps,
			-- holds, and receptors in this column will take 1 beat to go through
			-- their animations.
			taps= {
				NewSkinTapPart_Tap= {
					state_map= tap_state_map,
					-- If this noteskin used 3D notes, it would set the texture_map
					-- field instead of the state_map field:
					-- texture_map= tap_texture_map,
					actor= Def.Sprite{Texture= "tap_note 2x8.png",
						-- The Mask field sets the mask texture used for the tap.
						Mask= NEWSKIN:get_path(skin_name, "tap_mask 2x8.png"),
						-- Use the InitCommand to rotate the arrow appropriately.
						InitCommand= function(self) self:rotationz(rots[button]) end}},
				NewSkinTapPart_Mine= {
					state_map= mine_state_map,
					actor= Def.Sprite{Texture= "mine.png"}},
				NewSkinTapPart_Lift= {
					state_map= lift_state_map,
					actor= Def.Sprite{Texture= "tap_note 2x8.png",
						-- The Mask field sets the mask texture used for the tap.
						Mask= NEWSKIN:get_path(skin_name, "tap_mask 2x8.png"),
						-- Use the InitCommand to rotate the arrow appropriately.
						InitCommand= function(self) self:rotationz(rots[button]) end}},
			},
			-- Not used by this noteskin:  optional_taps.
			-- The optional_taps table is here in a comment as an example.  Since
			-- this noteskin does not use heads or tails, the notefield draws taps
			-- instead of heads and nothing for tails.
			-- optional_taps is the field used for heads and tails.  The elements
			-- in it are indexed by hold subtype and split into head and tail.
			-- When the notefield needs a head, it first looks for a head with the
			-- given subtype.  If that doesn't exist, it looks for the hold head.
			-- If that doesn't exist, it falls back on a normal tap.
			-- Tails fall back in a similar way, except they come up blank if the
			-- hold tail piece doesn't exist.
			--[[
			optional_taps= {
				-- This will be used if NewSkinTapOptionalPart_RollHead or
				-- NewSkinTapOptionalPart_CheckpointHead does not exist.
				NewSkinTapOptionalPart_HoldHead= {
					state_map= tap_state_map,
					-- Taps can have an inactive_state_map if they need to look
					-- different when the hold they're on is not active.
					inactive_state_map= tap_state_map,
					actor= Def.Sprite{Texture= "hold_head 2x8.png"}},
				-- This will be used if NewSkinTapOptionalPart_RollTail or
				-- NewSkinTapOptionalPart_CheckpointTail does not exist.
				NewSkinTapOptionalPart_HoldTail= {
					state_map= tap_state_map,
					actor= Def.Sprite{Texture= "hold_tail 2x8.png"}},
				NewSkinTapOptionalPart_RollHead= {
					state_map= tap_state_map,
					actor= Def.Sprite{Texture= "roll_head 2x8.png"}},
				NewSkinTapOptionalPart_RollTail= {
					state_map= tap_state_map,
					actor= Def.Sprite{Texture= "roll_tail 2x8.png"}},
				NewSkinTapOptionalPart_CheckpointHead= {
					state_map= tap_state_map,
					actor= Def.Sprite{Texture= "checkpoint_head 2x8.png"}},
				NewSkinTapOptionalPart_CheckpointTail= {
					state_map= tap_state_map,
					actor= Def.Sprite{Texture= "checkpoint_tail 2x8.png"}},
			},
			]]
			-- Also not used by this noteskin:
			--   reverse_taps and reverse_optional_taps.
			-- If reverse_taps exists, the taps in it will be used instead of the
			-- the taps in taps when the column is in reverse.
			-- If reverse_optional_taps exists, the taps in it will be used instead
			-- of the taps on optional_taps when the column is in reverse.
			holds= {
				-- The inactive states use the inactive_state_map while the active
				-- states use the active_state_map.  The state maps use different
				-- frames, so the active and inactive states look different while
				-- using the same texture.
				TapNoteSubType_Hold= {
					-- This is the quantized_hold for the inactive state of holds.
					{
						state_map= inactive_state_map,
						-- textures is a table so that the hold can be rendered as
						-- multiple layers.  The textures must all be the same size and
						-- frame dimensions.  The same frame from each will be rendered,
						-- in order.  This replaces the HoldActiveIsAddLayer flag that
						-- was in the old noteskin format.
						textures= {hold_tex},
						flip= hold_flips[button],
						-- If disable_filtering is set to true, then texture filtering
						-- will be turned off when rendering the hold.
						disable_filtering= false,
						-- The length_data table tells the system how long each part is.
						-- These are the default values, if length_data does not exist,
						-- or one of the values is not a number, the default value will
						-- be used.
						length_data= {
							-- start_note_offset is where to start drawing the hold body,
							-- relative to the note the head occurs at.
							start_note_offset= -.5,
							-- end_note_offset is where to stop drawing the hold body,
							-- relative to the note the hold ends at.
							end_note_offset= .5,
							-- head_pixs is how many pixels tall the head section of the
							-- texture is.
							head_pixs= 32,
							-- body_pixs is how many pixels tall one body section is.
							-- Remember that the body occurs twice, so this is only half
							-- the distance from the end of the head to the beginning of
							-- the tail.
							-- The body occurs twice so that part of the texture can be
							-- repeated when rendering a long hold.
							body_pixs= 64,
							-- tail_pixs is how many pixels tall the tail section of the
							-- texture is.
							tail_pixs= 32,
						},
					},
					-- This is the quantized_hold for the active state of holds.
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
			-- reverse_holds is the same as holds, but for when the reverse mod is
			-- being used.  Notice that it's using the same textures, but flipping
			-- them differently.
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
		}
	end
	return {
		columns= columns,
		-- In addition to each part having its own vivid flag, there is this
		-- global flag.  If this global flag is true or false, the flags for all
		-- the parts will be set to the same.  If you have different parts with
		-- different vivid flags, do not set vivid_operation.
		-- This noteskin isn't vivid in the traditional sense, but setting the
		-- vivid flag makes the notes at a different part of the beat start at a
		-- different part of the animation.  So the first 16th after the beat has
		-- the white outline, and the one before the next beat has the black
		-- outline.
		vivid_operation= true, -- output 200%
	}
end

-- There are two ways to support multiplayer mode:
-- 1. Mask mode.  The notefield applies a mask to taps and holds.  The alpha
--    channel in the mask sets where the color is applied.
-- 2. Quanta mode.  The notefield uses a different quanta for each player.
--    So the first quanta in the state map is for player 1, the second for
--    player 2, and so on.
-- The theme (or player) chooses which mode is used.  If the noteskin does
-- not supply masks for the holds, then the field will use Quanta mode when
-- the theme tries to use Mask mode.
--
-- If a noteskin wants to have holds that are playerized in multiplayer mode,
-- but not quantized in single player mode, then it should use the
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
-- If a noteskin wants to support masks for taps but not have masks on holds,
-- then it should use a hold mask that is transparent everywhere.
--
-- If the mask texture is not the same size as the texture for the thing
-- being masked (hold or tap), the mask texture will be stretched and look
-- wrong.
