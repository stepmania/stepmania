local function explosion_init(self)
	self:visible(false):SetAllStateDelays(.05)
end
local function explosion_hide(self)
	self:visible(false)
end
local judge_colors= {
	TapNoteScore_W2= {1, 1, .3, 1},
	TapNoteScore_W3= {0, 1, .4, 1},
	TapNoteScore_W4= {.3, .8, 1, 1},
	TapNoteScore_W5= {.8, 0, .6, 1},
	HoldNoteScore_Held= {1, 1, 1, 1},
}
return function(button_list)
	local rots= {ParaLeft= 90, ParaUpLeft= 135, ParaUp= 180, ParaUpRight= 225, ParaRight= 270}
	local ret= {}
	for i, button in ipairs(button_list) do
		ret[i]= Def.ActorFrame{
			InitCommand= function(self)
				self:draworder(notefield_draw_order.explosion)
					:rotationz(rots[button])
			end
			WidthSetCommand= function(self, param)
				param.column:set_layer_fade_type(self, "FieldLayerFadeType_Explosion")
			end,
			Def.Sprite{
				Texture= "_Down Tap Explosion Bright", InitCommand= explosion_init,
				ColumnJudgmentCommand= function(self, param)
					if not param.bright then return end
					if param.tap_note_score == "TapNoteScore_W1" then
						self:finishtweening():visible(true)
							:diffuse{1, 1, 1, 1}:zoom(1)
							:linear(.2):zoom(1.5)
							:decelerate(.1):zoom(1.5):diffusealpha(0)
							:sleep(0):queuecommand("hide")
					elseif judge_colors[param.tap_note_score] then
						self:finishtweening():visible(true)
							:diffuse(judge_colors[param.tap_note_score]):zoom(1)
							:linear(.06):zoom(1.1):linear(.06):diffusealpha(0)
							:sleep(0):queuecommand("hide")
					end
				end,
				hideCommand= explosion_hide,
			},
			Def.Sprite{
				Texture= "Down Tap Explosion Dim", InitCommand= explosion_init,
				ColumnJudgmentCommand= function(self, param)
					if param.bright then return end
					if param.tap_note_score == "TapNoteScore_W1" then
						self:finishtweening():visible(true)
							:diffuse{1, 1, 1, 1}:zoom(1)
							:linear(.2):zoom(1.5)
							:decelerate(.1):zoom(1.5):diffusealpha(0)
							:sleep(0):queuecommand("hide")
					elseif judge_colors[param.tap_note_score] then
						self:finishtweening():visible(true)
							:diffuse(judge_colors[param.tap_note_score]):zoom(1)
							:linear(.06):zoom(1.1):linear(.06):diffusealpha(0)
							:sleep(0):queuecommand("hide")
					end
				end,
				hideCommand= explosion_hide,
			},
			Def.Sprite{
				Texture= "Down Hold Explosion", InitCommand= explosion_init,
				ColumnJudgmentCommand= function(self, param)
					if param.hold_note_score == "HoldNoteScore_Held" then
						self:finishtweening():visible(true)
							:diffuse{1, 1, 1, 1}:zoom(1)
							:linear(.06):zoom(1.1)
							:linear(.06):diffusealpha(0)
							:sleep(0):queuecommand("hide")
					end
				end,
				hideCommand= explosion_hide,
			},
			Def.Sprite{
				Texture= "hit_mine_explosion", InitCommand= function(self)
					self:visible(false)
				end,
				ColumnJudgmentCommand= function(self, param)
					if param.tap_note_score == "TapNoteScore_HitMine" then
						self:visible(true):finishtweening()
							:blend("BlendMode_Add"):diffuse{1, 1, 1, 1}
							:zoom(1):rotationz(0):decelerate(.3):rotationz(90)
							:linear(.3):rotationz(180):diffusealpha(0)
							:sleep(0):queuecommand("hide")
					end
				end,
				hideCommand= function(self)
					self:visible(false)
				end,
			},
		}
	end
	return ret
end
