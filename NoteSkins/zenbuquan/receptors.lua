local column_width= 64
local function zoom_tap(self)
	self:zoom(column_width / 11)
		:draworder(notefield_draw_order.receptor)
end
local red= {1, 0, 0, 1}
local white= {1, 1, 1, 1}
return function(button_list, stepstype, skin_parameters)
	local ret= {}
	local warning_time= 1
	for i, button in ipairs(button_list) do
		ret[i]= Def.ActorFrame{
			InitCommand= zoom_tap,
			Def.Sprite{
				Texture= "receptor ", InitCommand= function(self)
					self:SetTextureFiltering(false)
				end,
				BeatUpdateCommand= function(self, param)
					self:setstate(math.floor(param.beat * self:GetNumStates()))
					if warning_time > 0 and param.second_distance < warning_time then
						self:diffuse(lerp_color(param.second_distance/warning_time, white, red))
					else
						self:diffuse(white)
					end
					if param.pressed then
						self:zoom(.75)
					elseif param.lifted then
						self:zoom(1)
					end
				end,
			}
		}
	end
	return ret
end
