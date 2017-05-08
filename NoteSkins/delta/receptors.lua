return function(button_list, stepstype, skin_parameters)
	local ret= {}
	local rots= {
		Left      = 90,
		Down      = 0,
		Up        = 180,
		Right     = 270,
		UpLeft    = 135,
		UpRight   = 225, 
		DownLeft  = 45,
		DownRight = 315
	}
	local texture = "Receptor 4x1.png"
	for i, button in ipairs(button_list) do
		ret[i] = Def.ActorFrame {
			InitCommand = function(self)
				self
					:rotationz(rots[button] or 0)
					:draworder(notefield_draw_order.receptor)
			end,
			WidthSetCommand = function(self, param)
				param.column:set_layer_fade_type(self, "FieldLayerFadeType_Receptor")
			end,
			Def.Sprite {
				Texture = texture,
				InitCommand = function(self)
					self:effectclock("beat")
					self:SetAllStateDelays(0.25)
				end,
				ColumnJudgmentCommand = function(self)
					self.none = false
				end,
				BeatUpdateCommand = function(self, param)
					if param.pressed then
						if self.none == true then
							if self.onepress == true then
								self:stoptweening():zoom(.75):linear(.11):zoom(1)
								self.onepress = false
							end
						end
					else
						self:zoom(1)
						self.onepress = true
						self.none = true
					end
				end,
			},
			Def.Sprite {
				Texture = texture,
				InitCommand = function(self)
					self
						:effectclock("beat")
						:blend("BlendMode_Add")
						:diffusealpha(0)
				end,
				BeatUpdateCommand = function(self, param)
					if param.pressed then
						self:diffusealpha(0.6)
					end
					if param.lifted then
						self:diffusealpha(0)
					end
				end
			}
		}
	end
	return ret
end
