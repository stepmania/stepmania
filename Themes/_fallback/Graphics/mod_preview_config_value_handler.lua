-- When you remake this file in your theme, make sure to give the parts the
-- same names.
return Def.ActorFrame{
	InitCommand= function(self)
		self:zoom(.5)
	end,
	Def.BitmapText{
		Name= "value_text", Font= "Common Normal", InitCommand= function(self)
			self:horizalign(right):x(-2):strokecolor{0, 0, 0, 1}
		end,
	},
	Def.BitmapText{
		Name= "name_text", Font= "Common Normal", InitCommand= function(self)
			self:horizalign(left):x(2):strokecolor{0, 0, 0, 1}
		end,
	},
	Def.Sprite{
		Name= "increase_button", Texture= THEME:GetPathG("", "up_button.png"),
		InitCommand= function(self)
			self:xy(-16, -24)
		end,
		ClickCommand= function(self)
			self:stoptweening()
				:linear(.1):zoom(.75)
				:linear(.1):zoom(1)
		end,
	},
	Def.Sprite{
		Name= "decrease_button", Texture= THEME:GetPathG("", "up_button.png"),
		InitCommand= function(self)
			self:xy(-16, 24):basezoomy(-1)
		end,
		ClickCommand= function(self)
			self:stoptweening()
				:linear(.1):zoom(.75)
				:linear(.1):zoom(1)
		end,
	},
}
