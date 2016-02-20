-- A layer file must return a function that takes a button list.  When the
-- noteskin is loaded, this function will be called with the list of buttons.
-- The function must return a table containing an actor for each column.
-- The notefield will set the x value of each column after loading.
-- These are full actors and their update functions will be called every
-- frame, so they do not have the limitations that notes have.
local red= {1, 0, 0, 1}
local white= {1, 1, 1, 1}
return function(button_list, stepstype, skin_parameters)
	local ret= {}
	local rots= {Left= 90, Down= 0, Up= 180, Right= 270}
	local warning_time= skin_parameters.receptors.warning_time
	for i, button in ipairs(button_list) do
		ret[i]= Def.Sprite{
			Texture= "receptor.png", InitCommand= function(self)
				self:rotationz(rots[button] or 0):effectclock("beat")
					:draworder(newfield_draw_order.receptor)
			end,
			WidthSetCommand= function(self, param)
				param.column:set_layer_fade_type(self, "FieldLayerFadeType_Receptor")
			end,
			-- The BeatUpdate command happens every frame to update various things
			-- that can change every frame.
			-- The param table has four things in it:
			--   beat: A number in the range [0, 1), it is 0 when the beat starts,
			--    	and goes towards 1 as the beat progresses.
			--   pressed: True if the panel for the column is pressed.
			--   beat_distance: The number of beats to the next note.
			--   second_distance: The number of seconds to the next note.
			-- The notefield has get_upcoming_time and set_upcoming_time functions
			-- for setting how far ahead it will look for a note for the
			-- beat_distance and second_distance fields.  If the next note is too
			-- far away, beat_distance and second_distance will be 1000.

			-- This example uses the beat field to calculate a glow value that
			-- starts at .25 and takes half a beat to fade to 0.
			-- A receptor that is based on animated frames could use
			-- SetSecondsIntoAnimation instead.

			-- The second_distance field is used to turn the receptor red when a
			-- note is 2 seconds away, then fade to white as the note approaches.
			-- warning_time is set by a noteskin parameter so the player can
			-- control the fading.
			BeatUpdateCommand= function(self, param)
				self:glow{1, 1, 1, (1 - param.beat*2) / 4}
				if warning_time > 0 and param.second_distance < warning_time then
					self:diffuse(lerp_color(param.second_distance/warning_time, white, red))
				else
					self:diffuse(white)
				end
				if param.pressed then
					self:zoom(.75)
				elseif param.lifted then
					self:zoom(1)
				end
			end,
		}
	end
	return ret
end
