-- Various functions to cover generic parts of making a noteskin.

NoteSkin= {
	-- common_multiple calculates the lowest common multiple of the factors.
	common_multiple= function(number, factors)
		for i= 1, #factors do
			if number % factors[i] ~= 0 then
				return false
			end
		end
		return true
	end,
	-- generic_state_map returns a state map where each quanta uses the same
	-- number of frames and the frames are in order.
	-- anim_frames is the number of animation frames each quanta has.
	-- per_beats is a table of how many times per beat each quanta occurs.
	-- Example:  The standard 8 quanta, each with 4 frames.
	--   Noteskin.generic_state_map(4, {1, 2, 3, 4, 6, 8, 12, 16})
	generic_state_map= function(anim_frames, per_beats)
		local largest_per_beat= 1
		for i= 1, #per_beats do
			largest_per_beat= math.max(per_beats[i], largest_per_beat)
		end
		local parts_per_beat= largest_per_beat
		while not NoteSkin.common_multiple(parts_per_beat, per_beats) do
			parts_per_beat= parts_per_beat + largest_per_beat
		end
		local quanta= {}
		local curr_frame= 1
		for q= 1, #per_beats do
			local states= {}
			for f= 1, anim_frames do
				states[f]= curr_frame
				curr_frame= curr_frame + 1
			end
			quanta[q]= {per_beat= per_beats[q], states= states}
		end
		return {parts_per_beat= parts_per_beat, quanta= quanta}
	end,
	-- single_quanta_state_map returns a state map where a single quanta uses
	-- all the frames given.  Useful for unquantized hold bodies.
	single_quanta_state_map= function(anim_frames)
		return {parts_per_beat= 1, quanta= {{per_beat= 1, states= anim_frames}}}
	end,
}
