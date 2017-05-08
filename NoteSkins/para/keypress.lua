return function(button_list)
	for i, button in ipairs(button_list) do
		ret[i]= Def.Sprite{
			Texture= "Down KeypressBlock", InitCommand= function(self)
				self:vertalign(top):zoomx(0)
					:draworder(notefield_draw_order.receptor-1)
			end,
			WidthSetCommand= function(self, param)
				param.column:set_layer_fade_type(self, "FieldLayerFadeType_Receptor")
				param.column:set_layer_transform_type(self, "FieldLayerTransformType_PosOnly")
			end,
			ReverseChangedCommand= function(self, param)
				if sign > 0 then
					self:vertalign(top)
				else
					self:vertalign(bottom)
				end
			end,
			BeatUpdateCommand= function(self, param)
				if param.pressed then
					self:finishtweening():zoomx(0):linear(.02):zoomx(1)
				elseif param.lifted then
					self:finishtweening():zoomx(1):linear(.14):zoomx(0)
				end,
			end,
		}
	end
	return ret
end
