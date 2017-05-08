local revert_sync_changes= THEME:GetString("ScreenSyncOverlay", "revert_sync_changes")
local change_bpm= THEME:GetString("ScreenSyncOverlay", "change_bpm")
local change_song_offset= THEME:GetString("ScreenSyncOverlay", "change_song_offset")
local change_machine_offset= THEME:GetString("ScreenSyncOverlay", "change_machine_offset")
local hold_alt= THEME:GetString("ScreenSyncOverlay", "hold_alt")

return Def.ActorFrame{
	Def.Quad{
		Name= "quad", InitCommand= function(self)
			self:diffuse{0, 0, 0, 0}:horizalign(right):vertalign(top)
				:xy(_screen.w, 0)
		end,
		ShowCommand= function(self)
			self:stoptweening():linear(.3):diffusealpha(.5)
				:sleep(4):linear(.3):diffusealpha(0)
		end,
		HideCommand= function(self)
			self:finishtweening()
		end,
	},
	Def.BitmapText{
		Name= "help_text", Font= "Common Normal", InitCommand= function(self)
			local text= {
				revert_sync_changes..":",
				"    F4",
				change_bpm..":",
				"    F9/F10",
				change_song_offset..":",
				"    F11/F12",
				change_machine_offset..":",
				"    Shift + F11/F12",
				hold_alt,
			}
			self:diffuse{1, 1, 1, 0}:horizalign(left):vertalign(top)
				:shadowlength(2):settext(table.concat(text, "\n"))
				:xy(_screen.w - self:GetZoomedWidth() - 10, 10)

			local quad= self:GetParent():GetChild("quad")
			quad:zoomtowidth(self:GetZoomedWidth()+20)
				:zoomtoheight(self:GetZoomedHeight()+20)
		end,
		ShowCommand= function(self)
			self:stoptweening():linear(.3):diffusealpha(1)
				:sleep(4):linear(.3):diffusealpha(0)
		end,
		HideCommand= function(self)
			self:finishtweening()
		end,
	},
	Def.BitmapText{
		Name= "Status", Font= "ScreenSyncOverlay status",
		InitCommand= function(self)
			ActorUtil.LoadAllCommands(self, "ScreenSyncOverlay")
			self:playcommand("On")
		end,
		SetStatusCommand= function(self, param)
			self:settext(param.text)
		end,
	},
	Def.BitmapText{
		Name= "Adjustments", Font= "ScreenSyncOverlay adjustments",
		InitCommand= function(self)
			ActorUtil.LoadAllCommands(self, "ScreenSyncOverlay")
			self:playcommand("On")
		end,
		SetAdjustmentsCommand= function(self, param)
			self:visible(param.visible):settext(param.text)
		end,
	},
}
