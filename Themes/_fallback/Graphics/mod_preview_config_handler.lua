-- Rewrite this file however you please to style it for your theme.
-- The actor names are are used to find and control certain actors.
-- If you leave out the actors for "max_offset", then the user will have no
-- buttons for "max_offset", but everything will still work fine.
-- So leave out any parts you don't want buttons for.
local button_area_width= 96
local button_area_height= 0 -- Set after positioning buttons.

local positions= {
	{lef, top},
	{rig, top},
	{rig, bot},
	{lef, bot},
}

local function calc_positions()
	local lef= 4
	local rig= _screen.w - (button_area_width) - 4
	local top= 4
	local bot= _screen.h - (button_area_height) - 4
	positions= {
		{lef, top},
		{rig, top},
		{rig, bot},
		{lef, bot},
	}
end

-- next_open_pos is changed by each actor as it initializes, so the actors
-- are automatically positioned, no matter how many or few there are.
local next_open_pos= 0

local function button_click(self)
	self:stoptweening()
		:linear(.1):zoom(.75)
		:linear(.1):zoom(1)
end

local function value_handler(name)
	return Def.ActorFrame{
		Name= name, InitCommand= function(self)
			local y= next_open_pos + 16
			next_open_pos= next_open_pos + 32
			self:zoom(.5):xy(24, y)
		end,
		Def.BitmapText{
			Name= "value_text", Font= "Common Normal", InitCommand= function(self)
				self:horizalign(right):x(-2):strokecolor{0, 0, 0, 1}
			end,
		},
		Def.BitmapText{
			Font= "Common Normal", InitCommand= function(self)
				self:horizalign(left):x(2):strokecolor{0, 0, 0, 1}
					:settext(THEME:GetString("EditModPreview", name))
			end,
		},
		Def.Sprite{
			Name= "increase_button", Texture= THEME:GetPathG("", "up_button.png"),
			InitCommand= function(self)
				self:xy(-16, -24)
			end,
			ClickCommand= button_click,
		},
		Def.Sprite{
			Name= "decrease_button", Texture= THEME:GetPathG("", "up_button.png"),
			InitCommand= function(self)
				self:xy(-16, 24):basezoomy(-1)
			end,
			ClickCommand= button_click,
		},
	}
end

local function bool_handler(name)
	return Def.ActorFrame{
		Name= name, InitCommand= function(self)
			local y= next_open_pos + 8
			next_open_pos= next_open_pos + 16
			self:zoom(.5):xy(16, y)
		end,
		Def.Sprite{
			Name= "checkbox",
			-- don't feel like drawing a checkbox.
			Texture= THEME:GetPathG("", "up_button.png"),
			InitCommand= function(self)
				self:rotationz(90):x(-10)
			end,
			-- SetCommand is used to initialize the actor.
			SetCommand= function(self, params)
				if params.value then
					self:rotationz(90)
				else
					self:rotationz(-90)
				end
			end,
			-- ClickCommand is used when the actor is clicked.
			ClickCommand= function(self, params)
				self:playcommand("Set", params)
			end,
		},
		Def.BitmapText{
			Font= "Common Normal", InitCommand= function(self)
				self:horizalign(left):x(2):strokecolor{0, 0, 0, 1}
					:settext(THEME:GetString("EditModPreview", name))
			end
		},
	}
end

return Def.ActorFrame{
	InitCommand= function(self)
		-- The size is for the area where the buttons are visible.
		button_area_height= next_open_pos
		calc_positions()
		self:GetChild("bg"):playcommand("Size")
		self:setsize(button_area_width, button_area_height)
	end,
	PositionCommand= function(self, params)
		local pid= ((params.corner-1)%#positions)+1
		self:xy(positions[pid][1], positions[pid][2])
	end,
	ShowCommand= function(self)
		self:visible(true)
	end,
	HideCommand= function(self)
		self:visible(false)
	end,
	Def.Quad{
		Name= "bg", SizeCommand= function(self)
			self:setsize(button_area_width, button_area_height)
				:diffuse{0, 0, 0, .5}
				:xy(button_area_width/2, button_area_height/2)
		end,
	},
	value_handler("max_offset"),
	value_handler("min_offset"),
	value_handler("playback_speed"),
	value_handler("zoom"),
	value_handler("corner"),
	bool_handler("paused"),
	bool_handler("show_preview"),
	bool_handler("full_screen"),
	bool_handler("reload_mods"),
}
