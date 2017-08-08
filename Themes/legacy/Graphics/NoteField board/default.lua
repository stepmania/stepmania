local filter_color= color("0.135,0.135,0.135,1")

local args= {
	Def.Quad{
		InitCommand= function(self)
			self:hibernate(math.huge):diffuse(filter_color)
		end,
		PlayerStateSetCommand= function(self, param)
			local pn= param.PlayerNumber
			local style= GAMESTATE:GetCurrentStyle(pn)
			local alf= getenv("ScreenFilter"..ToEnumShortString(pn)) or 0
			local width= style:GetWidth(pn) + 8
			self:setsize(width, _screen.h*4096):diffusealpha(alf):hibernate(0)
		end,
	}
}

if GAMESTATE:GetCurrentGame():GetName() == "kb7" then
	args[#args+1]= Def.Sprite{
		Texture= "board.png", InitCommand= function(self)
			self:y(_screen.cy):zoomy(_screen.h)
		end,
	}
end

return Def.ActorFrame(args)
