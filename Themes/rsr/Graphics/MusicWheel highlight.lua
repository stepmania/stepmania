local width = 330
local height = 46
local offset = -16

return Def.ActorFrame {
	Def.Quad {
		InitCommand=function(self)
			self:setsize(4,SCREEN_HEIGHT)
			self:x(-width/2+offset-2)
			self:diffuse({0, 0, 0, 0.7})
		end
	},
	Def.Quad {
		InitCommand=function(self)
			self:x(offset)
			self:setsize(width,height)
			self:blend('BlendMode_Add')
			--self:thump()
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