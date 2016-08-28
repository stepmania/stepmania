local fallback_item_params= {
	text_commands= {
		Font= "Common Normal", OnCommand= function(self)
			self:diffusealpha(0):linear(1):diffusealpha(1)
		end,
	},
	text_width= .7,
	value_text_commands= {
		Font= "Common Normal", OnCommand= function(self)
			self:diffusealpha(0):linear(1):diffusealpha(1)
		end,
	},
	value_image_commands= {
		OnCommand= function(self)
			self:diffusealpha(0):linear(1):diffusealpha(1)
		end,
	},
	value_width= .25,
	type_images= {
		bool= THEME:GetPathG("", "menu_icons/bool"),
		choice= THEME:GetPathG("", "menu_icons/bool"),
		menu= THEME:GetPathG("", "menu_icons/menu"),
	},
}

local menu_params= {
	x= _screen.w / 6, y= _screen.h * .01,
	width= _screen.w / 3, height= _screen.h * .95,
		translation_section= "editmode_options",
		num_displays= 1, el_height= 20,
		display_params= {
			el_zoom= .55, item_params= fallback_item_params, item_mt= nesty_items.value
		},
	}

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
	editor_notefield_menu(menu_params),
}

return frame
