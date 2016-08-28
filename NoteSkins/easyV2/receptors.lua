return function(button_list, stepstype, skin_parameters)
	local ret= {}
	local rots= {
		Left= 90, Down= 0, Up= 180, Right= 270,
		UpLeft= 90, UpRight= 180, 
		DownLeft= 0, DownRight= 270,  Center= 0
	}
	local tap_redir= {
		Left= "down", Right= "down", Down= "down", Up= "down", 
		UpLeft= "DownLeft", UpRight= "DownLeft", -- shared for dance and pump
		DownLeft= "DownLeft", DownRight= "DownLeft", Center= "Center"
	}
	for i, button in ipairs(button_list) do
		ret[i]= Def.Sprite{
			Texture= tap_redir[button].." receptor.png", InitCommand= function(self)
				self:rotationz(rots[button] or 0):effectclock("beat")
					:draworder(notefield_draw_order.receptor)
			end,
			WidthSetCommand= function(self, param)
				param.column:set_layer_fade_type(self, "FieldLayerFadeType_Receptor")
			end,
			ColumnJudgmentCommand= function(self, param)
				self.tap_note_score = param.tap_note_score
			end,
			BeatUpdateCommand= function(self, param)
				if param.pressed then
					if self.tap_note_score == nil or string.find(self.tap_note_score, "miss") or string.find(self.tap_note_score, "W4") then
						if self.onepress then
							self:stoptweening():zoom(.75):linear(.11):zoom(1)
							self.onepress = false
						end
					end
				elseif param.lifted then
					self:zoom(1)
					self.onepress = true
					self.tap_note_score = nil
				end      
			end,
		}
	end
	return ret
end
