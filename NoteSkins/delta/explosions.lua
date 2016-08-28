return function(button_list, stepstype, skin_params)
	local ret = {}
	local rots  = {
		Left      = 90,
		Down      = 0,
		Up        = 180,
		Right     = 270,
		UpLeft    = 135,
		UpRight   = 225, 
		DownLeft  = 45,
		DownRight = 315
	}
	local tap_redir= {}
	for i, button in ipairs(button_list) do
		ret[i] = Def.ActorFrame {
			InitCommand = function(self)
				self:rotationz(rots[button] or 0):draworder(notefield_draw_order.explosion)
			end,
			WidthSetCommand = function(self, param)
				param.column:set_layer_fade_type(self, "FieldLayerFadeType_Explosion")
			end,
			Def.Sprite {
				Texture = "Down explosion.png",
				InitCommand = function(self)
					self:diffusealpha(0)
				end,
				ColumnJudgmentCommand= function(self, param)
					local diffuse = {
						TapNoteScore_W1    = { 1.0, 1.0, 1.0, 1 },
						TapNoteScore_W2    = { 1.0, 1.0, 0.9, 1 },
						TapNoteScore_W3    = { 0.6, 1.0, 0.8, 1 },
						TapNoteScore_W4    = { 0.6, 1.0, 1.0, 1 },
						TapNoteScore_W5    = { 1.0, 0.6, 1.0, 1 },
						HoldNoteScore_Held = { 1.0, 1.0, 0.7, 1 },
					}
					local exp_color= diffuse[param.tap_note_score or param.hold_note_score]
					if exp_color then
						self:stoptweening()
							:diffuse(exp_color)
							:sleep(0.1)
							:decelerate(0.4)
							:diffusealpha(0)
					end
				end
			}
		}
	end
	return ret
end
