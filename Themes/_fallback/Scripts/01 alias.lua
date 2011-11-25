--[[ command aliases ]]

-- (this probably belongs in Sprite.lua)
function Sprite:cropto(w,h)
	self:CropTo(w,h)
end

-- (this probably belongs in Actor.lua)
function Actor:SetSize(w,h)
	self:setsize(w,h)
end

-- shorthand! this is tedious to type and makes things ugly so let's make it shorter.
-- _screen.w, _screen.h, etc.
_screen = {
	w  = SCREEN_WIDTH,
	h  = SCREEN_HEIGHT,
	cx = SCREEN_CENTER_X,
	cy = SCREEN_CENTER_Y
}

--[[ compatibility aliases ]]

--[[ Actor ]]
function Actor:hidden(bHide)
	Warn("hidden is deprecated, use visible instead. (used on ".. self:GetName() ..")")
	self:visible(not bHide)
end

--[[ ActorScroller: all of these got renamed, so alias the lowercase ones if
themes are going to look for them. ]]
ActorScroller.getsecondtodestination = ActorScroller.GetSecondsToDestination
ActorScroller.setsecondsperitem = ActorScroller.SetSecondsPerItem
ActorScroller.setnumsubdivisions = ActorScroller.SetNumSubdivisions
ActorScroller.scrollthroughallitems = ActorScroller.ScrollThroughAllItems
ActorScroller.scrollwithpadding = ActorScroller.ScrollWithPadding
ActorScroller.setfastcatchup = ActorScroller.SetFastCatchup

--[[ MenuTimer: just some case changes. ]]
MenuTimer.setseconds = MenuTimer.SetSeconds

--[[ GameState ]]
--[[ Aliases for old GAMESTATE timing functions. These have been converted to
SongPosition, but most themes still use these old functions. ]]
function GameState:GetSongBeat() return self:GetSongPosition():GetSongBeat() end
function GameState:GetSongBeatNoOffset() return self:GetSongPosition():GetSongBeatNoOffset() end
function GameState:GetSongBPS() return self:GetSongPosition():GetCurBPS() end
function GameState:GetSongDelay() return self:GetSongPosition():GetDelay() end
function GameState:GetSongFreeze() return self:GetSongPosition():GetFreeze() end

--[[ 3.9 Conditionals ]]
Condition = {
	Hour = function() return Hour() end,
	IsDemonstration = function() return GAMESTATE:IsDemonstration() end,
	CurSong = function(sSongName)
		return GAMESTATE:GetCurrentSong():GetDisplayMainTitle() == sSongName
	end,
	DayOfMonth = function() return DayOfMonth() end,
	MonthOfYear = function() return MonthOfYear() end,
	UsingModifier = function(pnPlayer, sModifier)
		return GAMESTATE:PlayerIsUsingModifier( pnPlayer, sModifier );
	end,
}