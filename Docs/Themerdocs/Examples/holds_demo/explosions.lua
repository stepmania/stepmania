local hold_colors= {
	TapNoteSubType_Hold= {1, 1, 0, 1},
	TapNoteSubType_Roll= {1, 0, 1, 1},
	-- At some point in the distant future, checkpoint (pump style) holds will
	-- be implemented as a hold subtype.
	TapNoteSubType_Checkpoint= {0, 1, 1, 1},
}

return function(button_list, stepstype, skin_parameters)
	local ret= {}
	local function make_receptor(texture)
		return Def.Sprite{
			Texture= texture, InitCommand= function(self)
				self:draworder(notefield_draw_order.explosion)
			end,
			WidthSetCommand= function(self, param)
				param.column:set_layer_fade_type(self, "FieldLayerFadeType_Explosion")
			end,
			ColumnJudgmentCommand= function(self, param)
				local dim_diffuse= {
					TapNoteScore_W1= {1, 1, 1, 1},
					TapNoteScore_W2= {1, 1, .85, 1},
					TapNoteScore_W3= {.5, 1, .75, 1},
					TapNoteScore_W4= {.5, 1, 1, 1},
					TapNoteScore_W5= {1, .5, 1, 1},
				}
				local bright_diffuse= {
					TapNoteScore_W1= {1, 1, 1, 1},
					TapNoteScore_W2= {1, 1, .9, 1},
					TapNoteScore_W3= {.6, 1, .8, 1},
					TapNoteScore_W4= {.6, 1, 1, 1},
					TapNoteScore_W5= {1, .6, 1, 1},
				}
				local score= param.tap_note_score
				local exp_color= dim_diffuse[score]
				if param.bright then
					exp_color= bright_diffuse[score]
				end
				if exp_color then
					-- The hide at the end is queued so that if the notefield applies
					-- glow to the explosion, it will still disappear at then end.
					self:visible(true):finishtweening():diffuse(exp_color):sleep(.1)
						:decelerate(.5):diffusealpha(0)
						:sleep(0):queuecommand("hide")
				end
			end,
			hideCommand= function(self)
				self:visible(false)
			end,
			HoldCommand= function(self, param)
				if hold_colors[param.type] then
					if param.start then
						self:finishtweening()
							:diffusealpha(1):glowshift()
							:effectcolor1(hold_colors[param.type])
							:effectcolor2{1, 1, 1, .7}:effectperiod(.1)
					elseif param.finished then
						self:stopeffect():diffusealpha(0)
					else
						self:zoom(param.life * 1.25)
					end
				end
			end,
		}
	end
	local texture= skin_parameters.note_style .. "_receptor"
	for i, button in ipairs(button_list) do
		ret[i]= make_receptor(texture)
	end
	return ret
end
