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
return function(button_list, stepstype, skin_parameters)
	-- This example doesn't use the stepstype arg, but it is provided in case
	-- someone needs it.

	-- rots is a convenience conversion table to easily take care of buttons
	-- that should be rotated.
	local rots= {Left= 90, Down= 0, Up= 180, Right= 270, UpLeft= 135, UpRight= 225, DownLeft= 45, DownRight= 315}
	local widths= {UpLeft= 46, UpRight= 46, DownLeft= 46, DownRight= 46}
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
	-- NoteSkinPart in each column is allowed to have its own state map, to allow
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
	local mine_state_map= NoteSkin.single_quanta_state_map{1, 2, 3, 4, 5, 6, 7, 8}

	-- Holds have active and inactive states, so they need a different state
	-- map.
	local hold_active_map= {}
	local hold_inactive_map= {}
	local hold_tex= ""
	local roll_tex= ""
	if skin_parameters.quantize_holds then
		hold_tex= "quant_hold"
		roll_tex= "quant_roll"
		hold_active_map= {
			parts_per_beat= parts_per_beat, quanta= {
				{per_beat= 1, states= {2}}, -- 4th
				{per_beat= 2, states= {4}}, -- 8th
				{per_beat= 3, states= {6}}, -- 12th
				{per_beat= 4, states= {8}}, -- 16th
				{per_beat= 6, states= {10}}, -- 24th
				{per_beat= 8, states= {12}}, -- 32nd
				{per_beat= 12, states= {14}}, -- 48th
				{per_beat= 16, states= {16}}, -- 64th
			},
		}
		hold_inactive_map= {
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
	else
		hold_tex= "unquant_hold"
		roll_tex= "unquant_roll"
		hold_active_map= NoteSkin.single_quanta_state_map{2}
		hold_inactive_map= NoteSkin.single_quanta_state_map{1}
	end
	-- The length_data table tells the system how long each part is.
	-- These are the default values, if length_data does not exist,
	-- or one of the values is not a number, the default value will
	-- be used.
	local hold_length= {
		-- See Docs/Themerdocs/5.1_incompatibilities/noteskin_explanation_pics/
		-- for pictures.
		-- Docs/Themerdocs/Examples/holds_demo is another example noteskin,
		-- showing different variants of using the hold length fields.
		-- pixels_before_note is the number of pixels between the top of the hold
		-- texture and where the note should be.
		pixels_before_note= 32,
		-- topcap_pixels is the number of pixels in the texture before the body
		-- section that will be repeated.
		-- If topcap_pixels is greater than pixels_before_note, then part of the
		-- topcap will be after the note.
		topcap_pixels= 32,
		-- body_pixels is the length of the body section.
		-- If the body of your hold has a repeating pattern instead of being a
		-- simple color, you need two body sections and body_pixels is the length
		-- of one section.
		body_pixels= 64,
		-- bottomcap_pixels is the number of pixels after the body section.
		-- Similar to topcap_pixels, if bottomcap_pixels is greater than
		-- pixels_after_note, then that part of the bottomcap will be drawn
		-- above the row the hold ends on.
		bottomcap_pixels= 32,
		-- pixels_after_note is similar to pixels_before_note.
		pixels_after_note= 32,
		-- If your hold body is just a simple color and does not vary along its
		-- length, then it does not need two body sections.
		-- This reduces the number of verts the engine needs to render, because
		-- of how the texture is repeated.
		-- needs_jumpback defaults to true.
		needs_jumpback= false,

		-- If the hold texture is marked as doubleres, then all pixel numbers
		-- need to be half the pixel height each part has in the texture.

		-- start_note_offset, end_note_offset, head_pixs, body_pixs, and
		-- tail_pixs are translated into the equivalent values in the new system,
		-- so noteskins made for 5.1.-3 should work the same as before.
	}

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
		-- There will be a section about mask textures and routine mode at the
		-- end of the file.  Ignore mask textures and don't include them in your
		-- noteskin if there's already too much to understand.
		columns[i]= {
			-- The width parameter specifies how wide the column is in pixels.
			-- Different columns are allowed to be different widths.
			-- Holds in this column will be stretched to this width.
			-- If the width is not provided by the noteskin, the default of 64 will
			-- be used.
			width= widths[button] or 64,
			-- The padding is the distance between this column and the adjacent
			-- column.  A padding value of 2 means 1 pixel on each side.
			-- The default is 0.
			padding= 0,
			-- If custom_x is set to a number, this column will be placed at that
			-- position instead of automatically positioned by the field.
			-- custom_x= 0,
			-- hold_gray_percent controls how far towards black a released hold
			-- turns.  0 means the hold turns completely black.
			hold_gray_percent= .25,
			-- If a roll and a hold start on the same row as a tap, and
			-- use_hold_heads_for_taps_on_row is true, the tap will be a roll head.
			use_hold_heads_for_taps_on_row= false,
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
				NoteSkinTapPart_Tap= {
					state_map= tap_state_map,
					-- If this noteskin used 3D notes, it would set the texture_map
					-- field instead of the state_map field:
					-- texture_map= tap_texture_map,
					actor= Def.Sprite{Texture= "tap_note",
						-- Use the InitCommand to rotate the arrow appropriately.
						InitCommand= function(self) self:rotationz(rots[button]) end}},
				NoteSkinTapPart_Mine= {
					state_map= mine_state_map,
					actor= Def.Sprite{Texture= "mine",}},
				NoteSkinTapPart_Lift= {
					state_map= lift_state_map,
					actor= Def.Sprite{Texture= "tap_note",
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
				-- This will be used if NoteSkinTapOptionalPart_RollHead or
				-- NoteSkinTapOptionalPart_CheckpointHead does not exist.
				NoteSkinTapOptionalPart_HoldHead= {
					state_map= tap_state_map,
					-- Taps can have an inactive_state_map if they need to look
					-- different when the hold they're on is not active.
					inactive_state_map= tap_state_map,
					actor= Def.Sprite{Texture= "hold_head 2x8.png"}},
				-- This will be used if NoteSkinTapOptionalPart_RollTail or
				-- NoteSkinTapOptionalPart_CheckpointTail does not exist.
				NoteSkinTapOptionalPart_HoldTail= {
					state_map= tap_state_map,
					actor= Def.Sprite{Texture= "hold_tail 2x8.png"}},
				NoteSkinTapOptionalPart_RollHead= {
					state_map= tap_state_map,
					actor= Def.Sprite{Texture= "roll_head 2x8.png"}},
				NoteSkinTapOptionalPart_RollTail= {
					state_map= tap_state_map,
					actor= Def.Sprite{Texture= "roll_tail 2x8.png"}},
				NoteSkinTapOptionalPart_CheckpointHead= {
					state_map= tap_state_map,
					actor= Def.Sprite{Texture= "checkpoint_head 2x8.png"}},
				NoteSkinTapOptionalPart_CheckpointTail= {
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
						state_map= hold_inactive_map,
						-- textures is a table so that the hold can be rendered as
						-- multiple layers.  The textures must all be the same size and
						-- frame dimensions.  The same frame from each will be rendered,
						-- in order.  This replaces the HoldActiveIsAddLayer flag that
						-- was in the old noteskin format.
						textures= {hold_tex},
						-- The flip field can be set to TexCoordFlipMode_X,
						-- TexCoordFlipMode_Y, or TexCoordFlipMode_XY to flip the
						-- texture.  The default is TexCoordFlipMode_None.
						flip= "TexCoordFlipMode_None",
						-- If disable_filtering is set to true, then texture filtering
						-- will be turned off when rendering the hold.
						disable_filtering= false,
						length_data= hold_length,
					},
					-- This is the quantized_hold for the active state of holds.
					{
						state_map= hold_active_map,
						textures= {hold_tex},
						length_data= hold_length,
					},
				},
				TapNoteSubType_Roll= {
					{
						state_map= hold_inactive_map,
						textures= {roll_tex},
						length_data= hold_length,
					},
					{
						state_map= hold_active_map,
						textures= {roll_tex},
						length_data= hold_length,
					},
				},
			},
		}
		-- reverse_holds is the same as holds, but for when the reverse mod is
		-- being used.
		columns[i].reverse_holds= columns[i].holds
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
