local frame= Def.ActorFrame{
	InitCommand= function(self) self:diffusealpha(0) end,
	ShowMenuCommand= function(self, params)
		self:stoptweening():diffusealpha(0):accelerate(.2):diffusealpha(1)
	end,
	HideMenuCommand= function(self)
		self:stoptweening():diffusealpha(1):accelerate(.2):diffusealpha(0)
	end,
	Def.Quad{
		InitCommand= function(self)
			self:xy(_screen.w / 6, _screen.cy):setsize(_screen.w / 3, _screen.h)
				:diffuse{0, 0, 0, .75}
		end,
	},
	editor_notefield_menu(LoadActor(THEME:GetPathG("", "generic_menu.lua"), 1, _screen.w/3, _screen.h*.95, .55, _screen.w*.01, _screen.h*.01)),
}

return frame
