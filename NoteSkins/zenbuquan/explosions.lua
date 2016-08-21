local column_width= 64
local function zoom_tap(self)
	self:zoom(column_width / 11)
		:draworder(notefield_draw_order.explosion)
end
local hold_colors= {
	TapNoteSubType_Hold= {1, 1, 0, 1},
	TapNoteSubType_Roll= {1, 0, 1, 1},
	TapNoteSubType_Checkpoint= {0, 1, 1, 1},
}
local white= {1, 1, 1, 1}
local function explosion_init(self)
	self:visible(false):SetAllStateDelays(.05):SetTextureFiltering(false)
end
local function explosion_hide(self)
	self:visible(false)
end
return function(button_list, stepstype)
	local ret= {}
	for i, button in ipairs(button_list) do
		ret[i]= Def.ActorFrame{
			InitCommand= zoom_tap,
			WidthSetCommand= function(self, param)
				param.column:set_layer_fade_type(self, "FieldLayerFadeType_Explosion")
			end,
			Def.Sprite{
				Texture= "explosion ", InitCommand= explosion_init,
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
						self:stoptweening()
							:diffuse(exp_color):zoom(1):diffusealpha(.9):visible(true)
							:linear(0.1):zoom(2):diffusealpha(.3)
							:linear(0.06):diffusealpha(0)
							:sleep(0):queuecommand("hide")
					end
				end,
				hideCommand= explosion_hide,
			},
			Def.Sprite{
				Texture= "explosion ", InitCommand= explosion_init,
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
				hideCommand= explosion_hide,
			}
		}
	end
	return ret
end
