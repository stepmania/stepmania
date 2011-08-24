return
{
	Init = function(self) return true end,
	Update = function(self,delta) return end,
	Exit = function(self) return end,

	CurrentSongChangedMessage = function(self, params)
		SCREENMAN:SystemMessageNoAnimate( GAMESTATE:GetCurrentSong():GetDisplayFullTitle() )
	end,
}
