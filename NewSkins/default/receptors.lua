-- A layer file must return a function that takes a button list.  When the
-- noteskin is loaded, this function will be called with the list of buttons.
-- The function must return a table containing an actor for each column.
-- The notefield will set the x value of each column after loading.
-- These are full actors and their update functions will be called every
-- frame, so they do not have the limitations that notes have.
local red= {1, 0, 0, 1}
local white= {1, 1, 1, 1}
return function(button_list, stepstype)
	local ret= {}
	local rots= {Left= 90, Down= 0, Up= 180, Right= 270}
	for i, button in ipairs(button_list) do
		ret[i]= Def.Sprite{
			Texture= "receptor.png", InitCommand= function(self)
				self:rotationz(rots[button] or 0):effectclock("beat")
			end,
			-- The Pressed command is the way to respond to the column being
			-- pressed.  The param table only has one element: "on".  If that
			-- element is true, the column is now pressed.
			PressedCommand= function(self, param)
			end,
			-- The Upcoming command happens every frame, to update the distance to
			-- the next note in the column.  There are two parameters:
			-- beat_distance and second_distance.  Both are measured from the
			-- receptors to the note.  beat_distance is the distance in beats, and
			-- second_distance is the distance in seconds.
			-- If the notefield does not detect a note within its draw distance,
			-- it sends a distance of 1000.
			-- The draw distance of the notefield varies with modifiers such as
			-- speed and boomerang, and might be controlled by lua.
			-- The SM 5.0.x notefield had ShowNoteUpcoming and HideNoteUpcoming
			-- commands with no distance information.
			UpcomingCommand= function(self, param)
			end,
			-- The BeatUpdate command happens every frame to update the per-beat
			-- animation.  The param table has one element, the beat value.  The
			-- beat value range is [0, 1), it is 0 when the beat occurs, and goes
			-- towards 1 as the beat progresses.
			-- This example uses that to calculate a glow value that starts at .25
			-- and takes half a beat to fade to 0.
			-- A receptor that is based on animated frames could use
			-- SetSecondsIntoAnimation instead.
			BeatUpdateCommand= function(self, param)
				self:glow{1, 1, 1, (1 - param.beat*2) / 4}
				if param.second_distance < 2 then
					self:diffuse(lerp_color(param.second_distance/2, white, red))
				else
					self:diffuse(white)
				end
				if param.pressed then
					self:zoom(.75)
				else
					self:zoom(1)
				end
			end
		}
	end
	return ret
end
