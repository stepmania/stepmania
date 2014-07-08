return Def.ActorFrame{
	Def.Quad {
		Name= "erp",
		InitCommand= function(self)
			self:setsize(SCREEN_WIDTH, SCREEN_HEIGHT)
			self:horizalign(left)
			self:vertalign(top)
			self:diffuse(color("0,0,0,0"))
			self:diffusealpha(.85)
		end,
		SetCoveredHeightCommand= function(self, param)
		end
	}
}
