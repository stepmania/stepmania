local filter_color= color("0,0,0,1")
local this_pn= false

local layers= {
	Def.Quad{
		InitCommand= function(self)
			self:hibernate(math.huge):draworder(notefield_draw_order.board)
				:diffuse(filter_color):SetHeight(4096)
		end,
		PlayerStateSetCommand= function(self, param)
			this_pn= param.PlayerNumber
			local pn= param.PlayerNumber
			local alf= player_config:get_data(pn).ScreenFilter
			if alf > 0 then
				self:diffusealpha(alf):hibernate(0)
			end
		end,
		WidthSetCommand= function(self, param)
			self:SetWidth(param.width+8)
		end,
		MenuValueChangedMessageCommand= function(self, param)
			if param.pn ~= this_pn then return end
			if param.category ~= "Config" then return end
			if param.config_name ~= player_config.name then return end
			local alf= player_config:get_data(param.pn).ScreenFilter
			if alf > 0 then
				self:diffusealpha(alf):hibernate(0)
			else
				self:hibernate(math.huge)
			end
		end,
	}
}

return layers
