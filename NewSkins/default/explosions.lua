-- See the explanation in receptors.lua.

local hold_colors= {
	TapNoteSubType_Hold= {1, 1, 0, 1},
	TapNoteSubType_Roll= {1, 0, 1, 1},
	-- At some point in the distant future, checkpoint (pump style) holds will
	-- be implemented as a hold subtype.
	TapNoteSubType_Checkpoint= {0, 1, 1, 1},
}
local white= {1, 1, 1, 1}

return function(button_list, stepstype)
	local ret= {}
	local rots= {Left= 90, Down= 0, Up= 180, Right= 270}
	for i, button in ipairs(button_list) do
		ret[i]= Def.ActorFrame{
			InitCommand= function(self)
				self:rotationz(rots[button] or 0)
			end,
			Def.Sprite{
				Texture= "explosion.png", InitCommand= function(self)
					self:visible(false):SetAllStateDelays(.05)
				end,
				-- The ColumnJudgment command is the way to make the actor respond to
				-- a judgment that occurs in the column.  The param argument is a table
				-- with the information for the judgment.  These are the elements of
				-- the param table:
				-- {
				--   column, -- The index of the column.
				--   bright, -- True if the player's current combo is greater than or
				--           -- equal to the theme's BrightGhostComboThreshold metric.
				--   tap_note_score, -- The TapNoteScore value of the judgment.  If the
				--                   -- judgment is for a hold, this is nil.
				--   hold_note_score, -- The HoldNoteScore of the judgment.  If the
				--                    -- judgment is for a tap, this is nil.
				-- }
				-- This example uses the tap_note_score element to decide what color to
				-- use.
				-- Checking the column parameter isn't necessary because the command is
				-- only played on the column that it is for.
				ColumnJudgmentCommand= function(self, param)
					local diffuse= {
						TapNoteScore_W1= {1, 1, 1, 1},
						TapNoteScore_W2= {1, 1, .3, 1},
						TapNoteScore_W3= {0, 1, .4, 1},
						TapNoteScore_W4= {.3, .8, 1, 1},
						TapNoteScore_W5= {.8, 0, .6, 1},
						HoldNoteScore_Held= {1, 1, 1, 1},
					}
					local exp_color= diffuse[param.tap_note_score or param.hold_note_score]
					if exp_color then
						-- The hide at the end is queued so that if the notefield applies
						-- glow to the explosion, it will still disappear at then end.
						self:stoptweening()
							:diffuse(exp_color):zoom(1):diffusealpha(.9):visible(true)
							:linear(0.1):zoom(2):diffusealpha(.3)
							:linear(0.06):diffusealpha(0)
							:sleep(0):queuecommand("hide")
					end
				end,
				hideCommand= function(self)
					self:visible(false)
				end,
			},
			Def.Sprite{
				Texture= "explosion.png", InitCommand= function(self)
					self:visible(false):SetAllStateDelays(.05)
				end,
				-- The Hold command is the way to respond to a hold being active.
				-- When there is a hold in the column, the Hold command will be sent.
				-- The frame after a hold ends, it will be sent with nothing in param.
				-- The param argument has three elements:
				-- {
				--   type, -- A TapNoteSubType enum string.
				--   life, -- The life value of the hold.  When the hold is dropped,
				--         -- this will decrease from 1 to 0.
				--   start, -- True if this is the first frame of the hold.
				--   finished, -- True if the hold ended last frame.
				-- }
				-- This example picks a color based on the type.
				HoldCommand= function(self, param)
					if hold_colors[param.type] then
						if param.start then
							self:finishtweening()
								:zoom(1.25):diffuse(white):visible(true)
								:diffuseblink():effectcolor1(hold_colors[param.type])
								:effectcolor2(white):effectperiod(.1)
						elseif param.finished then
							self:stopeffect():linear(0.06):diffusealpha(0)
								:sleep(0):queuecommand("hide")
						else
							self:zoom(param.life * 1.25)
						end
					end
				end,
				hideCommand= function(self)
					self:visible(false)
				end,
			}
		}
	end
	return ret
end
