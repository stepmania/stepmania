-- This is an example for the toasty animation used by ScreenGameplay.
-- To try it out, put it in BGAnimations/ScreenGameplay toasty.lua

-- The toasty animation is loaded as a transition by ScreenGameplay, which
-- means that StartTransitioningCommand is played to activate it.
-- However, the details of the toasty are broadcast with the ToastyAchieved
-- message.  The message is broadcast before StartTransitioningCommand is
-- played, so we can set some internal variables to store the details.

local pn
local toasty_combo
local level

-- These is a table for storing player-specific values to use in the animation.

local rotations= { [PLAYER_1]= 15, [PLAYER_2]= -15 }
local positions= { [PLAYER_1]= _screen.cx*.5, [PLAYER_2]= _screen.cx*1.5 }
local colors= { [PLAYER_1]= PlayerColor(PLAYER_1), [PLAYER_2]= PlayerColor(PLAYER_2) }

return Def.BitmapText{
	Font= "Common Normal", Text= "Toasty!",
	InitCommand= function(self)
		self:zoom(0)
	end,
	ToastyAchievedMessageCommand= function(self, param)
		pn= param.PlayerNumber
		toasty_combo= param.ToastyCombo
		level= param.Level
	end,
	StartTransitioningCommand= function(self)
		-- The engine calls finishtweening when a transition is reset, and the
		-- toasty transition is reset before it's run, so we don't have to call
		-- finishtweening here.
		-- This is a simple animation that positions the text in the center of
		-- one half of the screen at zero zoom and rotation.  Then it tweens to
		-- a higher zoom and rotation, so it appears to spin out of nothing, then
		-- spin back down to nothing.
		self:xy(positions[pn], _screen.cy):zoom(0):rotationz(0):diffuse(colors[pn])
			:spring(level):zoom(level):rotationz(rotations[pn] * level)
			:spring(level):zoom(0):rotationz(0)
	end
}

-- Flaw:  When two players are playing, if they each get a toasty near the
-- same time, the animation will start playing for one, then switch to the
-- other.  As an exercise, try to figure out a way to make it play nicely for
-- multiple players earning a toasty at the same time.
