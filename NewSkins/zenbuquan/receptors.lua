local column_width= 64
local function zoom_tap(self)
	self:zoom(column_width / 11)
end
local red= {1, 0, 0, 1}
local white= {1, 1, 1, 1}
return function(button_list, stepstype)
	local ret= {}
	for i, button in ipairs(button_list) do
		ret[i]= Def.ActorFrame{
			InitCommand= zoom_tap,
			Def.Sprite{
				Texture= "receptor ", InitCommand= function(self)
					self:SetTextureFiltering(false)
				end,
				PressedCommand= function(self, param)
					if param.on then
						self:zoom(.75)
					else
						self:zoom(1)
					end
				end,
				UpcomingCommand= function(self, param)
					if param.second_distance < 2 then
						self:diffuse(lerp_color(param.second_distance/2, white, red))
					else
						self:diffuse(white)
					end
				end,
				BeatUpdateCommand= function(self, param)
					self:setstate(math.floor(param.beat * self:GetNumStates()))
				end
			}
		}
	end
	return ret
end
