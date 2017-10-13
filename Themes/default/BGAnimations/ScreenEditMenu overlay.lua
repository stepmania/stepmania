local steps_item_space= _screen.h*.05
local steps_item_width= steps_item_space * .75
local steps_type_item_space= _screen.h*.06

local steps_display= setmetatable({disable_wrapping= true}, item_scroller_mt)
local function steps_transform(self, item_index, num_items)
	self.container:xy((item_index-.5) * steps_item_space, 19)
end
local function stype_transform(self, item_index, num_items)
	self.container:y((item_index-1) * steps_type_item_space)
end
local stype_item_mt= edit_pick_menu_steps_display_item(
	stype_transform, Def.BitmapText{
		Font= "Common Condensed", InitCommand= function(self)
			self:zoom(0.8):horizalign(left):shadowlength(1)
		end,
		SetCommand= function(self, param)
			self:settext(THEME:GetString("LongStepsType", ToEnumShortString(param.stype)))
		end,
																 },
	steps_transform, Def.BitmapText{
		Font= "Common Fallback Font", InitCommand= function(self)
			self:zoom(0.75):shadowlength(1)
		end,
		SetCommand= function(self, param)
			self:settext(param.steps:GetMeter())
				:diffuse(ColorDarkTone(GameColor.Difficulty[param.steps:GetDifficulty()]))
			width_limit_text(self, steps_item_width, .5)
		end,
})

local picker_width= _screen.w * .5
local spacer= _screen.h * .07
local jacket_size= _screen.h * .2
local jacket_x= _screen.w - spacer - (jacket_size / 2)
local jacket_y= _screen.h * .26
local banner_width= _screen.w - picker_width - jacket_size - (spacer * 3)
local banner_height= _screen.h * .2
local banner_x= jacket_x - spacer - (jacket_size / 2) - (banner_width / 2)
local banner_y= jacket_y
local bpm_x= jacket_x + (jacket_size / 2)
local bpm_y= jacket_y + (jacket_size / 2) + (spacer / 2)
local length_x= bpm_x
local length_y= bpm_y + (spacer / 2)
local title_x= banner_x - (banner_width / 2)
local title_y= bpm_y
local artist_x= title_x
local artist_y= length_y
local steps_display_x= title_x
local steps_display_y= title_y + (spacer * 1.2)
local steps_display_items= (_screen.h - steps_display_y) / steps_type_item_space

-- Adding color, font, and shadowlength params to generic_menu.lua is left as
-- an exercise for the reader.  generic_menu.lua is meant to be customized
-- and different in each theme anyway.

-- Font: "Common Fallback Font"
-- Name color: color("#452429")
-- Value color: color("#6E2C34")

local frame= Def.ActorFrame{
	edit_menu_selection_changedMessageCommand=
		edit_pick_menu_update_steps_display_info(steps_display),
	edit_pick_menu_actor(LoadActor(THEME:GetPathG("", "generic_menu"), 1, _screen.cx-50, _screen.h-170, 1, 32, 125, 36)),
	Def.Sprite{
		Name= "jacket", InitCommand= function(self)
			self:xy(jacket_x, jacket_y)
		end,
		edit_menu_selection_changedMessageCommand= function(self, params)
			if params.group then
				self:visible(false)
			elseif params.song then
				if params.song:HasJacket() then
					self:visible(true)
					self:LoadBanner(params.song:GetJacketPath())
					scale_to_fit(self, jacket_size, jacket_size)
				else
					self:visible(false)
				end
			end
		end,
	},
	Def.Sprite{
		Name= "banner", InitCommand= function(self)
			self:xy(banner_x, banner_y)
		end,
		edit_menu_selection_changedMessageCommand= function(self, params)
			if params.group then
				local path= SONGMAN:GetSongGroupBannerPath(params.group)
				if #path > 0 then
					self:visible(true)
					self:LoadBanner(path)
					scale_to_fit(self, banner_width, banner_height)
				else
					self:visible(false)
				end
			elseif params.song then
				if params.song:HasBanner() then
					self:visible(true)
					self:LoadBanner(params.song:GetBannerPath())
					scale_to_fit(self, banner_width, banner_height)
				else
					self:visible(false)
				end
			end
		end,
	},
	Def.BitmapText{
		Name= "length", Font= "Common Fallback Font", InitCommand= function(self)
			self:xy(length_x, length_y):horizalign(right):zoom(.75):diffuse(color("#2E0F0F")):shadowlength(1)
		end,
		edit_menu_selection_changedMessageCommand= function(self, params)
			if params.group then
				self:visible(false)
			elseif params.song then
				self:settext(SecondsToMSS(params.song:MusicLengthSeconds()))
				self:visible(true)
			end
		end,
	},
	Def.BitmapText{
		Name= "bpm", Font= "Common Fallback Font", InitCommand= function(self)
			self:xy(bpm_x, bpm_y):horizalign(right):zoom(.75):diffuse(color("#451515")):shadowlength(1)
		end,
		edit_menu_selection_changedMessageCommand= function(self, params)
			if params.group then
				self:visible(false)
			elseif params.song then
				local display_bpm= params.song:GetDisplayBpms()
				if display_bpm[1] == display_bpm[2] then
					self:settextf("%d BPM", display_bpm[1])
				else
					self:settextf("%d - %d BPM", math.round(display_bpm[1]), math.round(display_bpm[2]))
				end
				self:visible(true)
			end
		end,
	},
	Def.BitmapText{
 		Name= "title", Font= "Common Fallback Font", InitCommand= function(self)
			self:xy(title_x, title_y):horizalign(left):zoom(.75):diffuse(color("#451515")):shadowlength(1)
		end,
		edit_menu_selection_changedMessageCommand= function(self, params)
			if params.group then
				self:visible(false)
			elseif params.song then
				self:settext(params.song:GetDisplayMainTitle())
				self:visible(true)
			end
		end,
	},
	Def.BitmapText{
 		Name= "artist", Font= "Common Fallback Font", InitCommand= function(self)
			self:xy(artist_x, artist_y):horizalign(left):zoom(.75):diffuse(color("#2E0F0F")):shadowlength(1)
		end,
		edit_menu_selection_changedMessageCommand= function(self, params)
			if params.group then
				self:visible(false)
			elseif params.song then
				self:settext(params.song:GetDisplayArtist())
				self:visible(true)
			end
		end,
	},
	steps_display:create_actors("steps_display", steps_display_items, stype_item_mt, steps_display_x, steps_display_y),
}

return frame
