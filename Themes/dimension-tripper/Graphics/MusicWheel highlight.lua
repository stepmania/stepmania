local Metric = function(c,k) return tonumber(THEME:GetMetric(c, k)) end
local width = Theme.MusicWheelItemWidth()
local height = Metric("MusicWheel", "ItemHeight")
local offset = Metric("MusicWheel", "ItemOffset")

return Def.ActorFrame {
	Def.Quad {
		InitCommand=function(self)
			self:x(offset)
			self:setsize(width,height)
			self:blend('BlendMode_Add')
			self:diffuse({1, 0.8, 0.7, 0.1})
			self:effectclock('beat')
		end,
		CurrentSongChangedMessageCommand=function(self, params)
			if not GAMESTATE:GetCurrentSong() then
				self:stopeffect()
			else
				self:thump()
			end
		end
	}
}