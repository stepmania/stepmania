local filter_color= color("0.135,0.135,0.135,1")

local layers= {
	Def.Quad{
		InitCommand= function(self)
			self:hibernate(math.huge):draworder(notefield_draw_order.board)
				:diffuse(filter_color)
		end,
		PlayerStateSetCommand= function(self, param)
			local pn= param.PlayerNumber
			local alf= player_config:get_data(pn).ScreenFilter
			if alf > 0 then
				self:SetHeight(4096):diffusealpha(alf):hibernate(0)
			end
		end,
		WidthSetCommand= function(self, param)
			self:SetWidth(param.width+8)
		end,
	}
}

return layers
