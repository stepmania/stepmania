return function(button_list, stepstype, skin_parameters)
	local ret= {}
	local function make_receptor(texture)
		return Def.Sprite{
			Texture= texture, InitCommand= function(self)
				self:draworder(notefield_draw_order.receptor)
			end,
			WidthSetCommand= function(self, param)
				param.column:set_layer_fade_type(self, "FieldLayerFadeType_Receptor")
			end,
			BeatUpdateCommand= function(self, param)
				self:glow{1, 1, 1, (1 - param.beat*2) / 4}
				if param.pressed then
					self:stoptweening():linear(.1):zoom(.8)
				elseif param.lifted then
					self:stoptweening():linear(.1):zoom(1)
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
