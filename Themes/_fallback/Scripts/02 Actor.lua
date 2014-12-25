-- Convenience aliases:
left = "HorizAlign_Left"
center = "HorizAlign_Center"
right = "HorizAlign_Right"
top = "VertAlign_Top"
middle = "VertAlign_Middle"
bottom = "VertAlign_Bottom"

function Actor:ease(t, fEase)
	-- Optimizations:
	-- fEase = -100 is equivalent to TweenType_Accelerate.
	if fEase == -100 then
		self:accelerate(t)
		return self
	end

	-- fEase = 0 is equivalent to TweenType_Linear.
	if fEase == 0 then
		self:linear(t)
		return self
	end

	-- fEase = +100 is equivalent to TweenType_Decelerate.
	if fEase == 100 then
		self:decelerate(t)
		return self
	end

	self:tween( t, "TweenType_Bezier",
		{
			0,
			scale(fEase, -100, 100, 0/3, 2/3),
			scale(fEase, -100, 100, 1/3, 3/3),
			1
		}
	)
	return self
end
-- Notes On Beziers --
-- They can be 1D ( Quadratic ) or 2D ( Bezier )
-- 1D:
--	XA XB YC YD
-- 2D:
--  XA XB XC XD YA YB YC YD
-- In 1D Quads, XA XB are beginning time and size, YC YD are ending time and size
-- In 2D Quads, X
local BounceBeginBezier =
{
	0, 0,
	0.42, -0.42,
	2/3, 0.3,
	1, 1
}
function Actor:bouncebegin(t)
	self:tween( t, "TweenType_Bezier", BounceBeginBezier )
	return self
end

local BounceEndBezier =
{
	0,0,
	1/3, 0.7,
	0.58, 1.42,
	1, 1
}
function Actor:bounceend(t)
	self:tween( t, "TweenType_Bezier", BounceEndBezier )
	return self
end

local SmoothBezier =
{
	0, 0, 1, 1
}
function Actor:smooth(t)
	self:tween( t, "TweenType_Bezier", SmoothBezier )
	return self
end
-- SSC Additions
local DropBezier =
{
	0	,	0,
	1/3	,	1,
	2/3	,	0.5,
	1	,	1,
}
function Actor:drop(t)
	self:tween( t, "TweenType_Bezier", DropBezier )
	return self
end

-- compound tweens "combine multiple interpolators to allow generating more
-- complex tweens." length is how long to span the animation for, while
-- ... is either a string (e.g. "linear,0.25,accelerate,0.75") or a table
-- with the tween information.
function Actor:compound(length,...)
	local tweens = ...

	if type(tweens) == "string" then
		local parsed = split(";",tweens)
		tweens = {} -- convert to table
		for i,s in pairs(parsed) do
			local res = split(",",s)

			tweens[i] = {
				Type = res[1],
				Percent = res[2],
				Bezier = res[3] or nil
			}
		end
	end

	for i,t in pairs(tweens) do
		if t.Type == "linear" then self:linear(t.Percent*length)
		elseif t.Type == "accelerate" then self:accelerate(t.Percent*length)
		elseif t.Type == "decelerate" then self:decelerate(t.Percent*length)
		elseif t.Type == "spring" then self:spring(t.Percent*length)
		elseif t.Type == "bouncebegin" then self:bouncebegin(t.Percent*length)
		elseif t.Type == "bounceend" then self:bounceend(t.Percent*length)
		elseif t.Type == "smooth" then self:smooth(t.Percent*length)
		elseif t.Type == "drop" then self:smooth(t.Percent*length)
		--elseif t.Type == "ease" then self:ease(t.Percent*length)
		elseif t.Type == "bezier" then
			-- todo: handle using tween and 'TweenType_Bezier'
		end
	end
	return self
end

-- Hide if b is true, but don't unhide if b is false.
function Actor:hide_if(b)
	if b then
		self:visible(false)
	end
	return self
end

function Actor:player(p)
	self:visible( GAMESTATE:IsHumanPlayer(p) )
	return self
end

function ActorFrame:propagatecommand(...)
	self:propagate(1)
	self:playcommand(...)
	self:propagate(0)
	return self
end

-- Shortcut for alignment.
--   cmd(align,0.5,0.5)  -- align center
--   cmd(align,0.0,0.0)  -- align top-left
--   cmd(align,0.5,0.0)  -- align top-center
function Actor:align(h, v)
	self:halign( h )
	self:valign( v )
	return self
end

function Actor:FullScreen()
	self:stretchto( 0,0,SCREEN_WIDTH,SCREEN_HEIGHT )
	return self
end

bg_fit_functions= {
	BackgroundFitMode_CoverDistort= function(self, width, height)
		local xscale= width / self:GetWidth()
		local yscale= height / self:GetHeight()
		self:zoomx(xscale)
		self:zoomy(yscale)
	end,
	BackgroundFitMode_CoverPreserve= function(self, width, height)
		local xscale= width / self:GetWidth()
		local yscale= height / self:GetHeight()
		self:zoom(math.max(xscale, yscale))
	end,
	BackgroundFitMode_FitInside= function(self, width, height)
		local xscale= width / self:GetWidth()
		local yscale= height / self:GetHeight()
		self:zoom(math.min(xscale, yscale))
	end,
	BackgroundFitMode_FitInsideAvoidLetter= function(self, width, height)
		local yscale= height / self:GetHeight()
		self:zoom(yscale)
	end,
	BackgroundFitMode_FitInsideAvoidPillar= function(self, width, height)
		local xscale= width / self:GetWidth()
		self:zoom(xscale)
	end
}

function Actor:scale_or_crop_background_no_move()
	local fit_mode= PREFSMAN:GetPreference("BackgroundFitMode")
	if bg_fit_functions[fit_mode] then
		bg_fit_functions[fit_mode](self, SCREEN_WIDTH, SCREEN_HEIGHT)
	else
		self:scaletocover(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT)
	end
	return self
end

function Actor:scale_or_crop_background()
	self:scale_or_crop_background_no_move()
	self:xy(SCREEN_CENTER_X, SCREEN_CENTER_Y)
	return self
end

function Actor:CenterX() self:x(SCREEN_CENTER_X) return self end
function Actor:CenterY() self:y(SCREEN_CENTER_Y) return self end
function Actor:Center() self:xy(SCREEN_CENTER_X,SCREEN_CENTER_Y) return self end

function Actor:bezier(...)
	local a = {...}
	local b = {}
	local c = 0
	assert((a == 9 or a == 5), "bad number of arguments for Actor:bezier()")
	for i=3,c do
		b[#b+1] = a[i]
	end
	self:tween(a[2], "TweenMode_Bezier", b)
	return self
end

function Actor:Real()
	-- scale back down to real pixels.
	self:basezoom(GetReal())
	-- don't make this ugly
	self:SetTextureFiltering(false)
	return self
end

-- Scale things back up after they have already been scaled down.
function Actor:RealInverse()
	-- scale back up to theme resolution
	self:basezoom(GetRealInverse())
	self:SetTextureFiltering(true)
	return self
end

-- MaskSource([clearzbuffer])
-- Sets an actor up as the source for a mask. Clears zBuffer by default.
function Actor:MaskSource(noclear)
	if noclear == true then
		self:clearzbuffer(true)
	end
	self:zwrite(true)
	self:blend('BlendMode_NoEffect')
	return self
end

-- MaskDest()
-- Sets an actor up to be masked by anything with MaskSource().
function Actor:MaskDest()
	self:ztest(true)
	return self
end

-- Thump()
-- A customized version of pulse that is more appealing for on-beat
-- effects;
function Actor:thump(fEffectPeriod)
	self:pulse()
	if fEffectPeriod ~= nil then
		self:effecttiming(0,0,0.75*fEffectPeriod,0.25*fEffectPeriod)
	else
		self:effecttiming(0,0,0.75,0.25)
	end
	-- The default effectmagnitude will make this effect look very bad.
	self:effectmagnitude(1,1.125,1)
	return self
end

-- Heartbeat()
-- A customized version of pulse that is more appealing for on-beat
-- effects;
function Actor:heartbeat(fEffectPeriod)
	self:pulse()
	if fEffectPeriod ~= nil then
		self:effecttiming(0,0.125*fEffectPeriod,0.125*fEffectPeriod,0.75*fEffectPeriod);
	else
		self:effecttiming(0,0.125,0.125,0.75);
	end
	self:effectmagnitude(1,1.125,1)
	return self
end

--[[ BitmapText commands ]]

-- PixelFont()
-- An alias that turns off texture filtering.
-- Named because it works best with pixel fonts.
function BitmapText:PixelFont()
	self:SetTextureFiltering(false)
	return self
end

-- Stroke(color)
-- Sets the text's stroke color.
function BitmapText:Stroke(c)
	self:strokecolor( c )
	return self
end

-- NoStroke()
-- Removes any stroke.
function BitmapText:NoStroke()
	self:strokecolor( color("0,0,0,0") )
	return self
end

-- Set Text With Format (contributed by Daisuke Master)
-- this function is my hero - shake
function BitmapText:settextf(...)
	self:settext(string.format(...))
	return self
end

-- DiffuseAndStroke(diffuse,stroke)
-- Set diffuse and stroke at the same time.
function BitmapText:DiffuseAndStroke(diffuseC,strokeC)
	self:diffuse(diffuseC)
	self:strokecolor(strokeC)
	return self
end;
--[[ end BitmapText commands ]]

function Actor:LyricCommand(side)
	self:settext( Var "LyricText" )
	self:draworder(102)

	self:stoptweening()
	self:shadowlengthx(0)
	self:shadowlengthy(5)
	self:strokecolor(color("#000000"))

	local Zoom = SCREEN_WIDTH / (self:GetZoomedWidth()+1)
	if( Zoom > 1 ) then
		Zoom = 1
	end
	self:zoomx( Zoom )

	local lyricColor = Var "LyricColor"
	local Factor = 1
	if side == "Back" then
		Factor = 0.5
	elseif side == "Front" then
		Factor = 0.9
	end
	self:diffuse( {
		lyricColor[1] * Factor,
		lyricColor[2] * Factor,
		lyricColor[3] * Factor,
		lyricColor[4] * Factor } )

	if side == "Front" then
		self:cropright(1)
	else
		self:cropleft(0)
	end

	self:diffusealpha(0)
	self:linear(0.2)
	self:diffusealpha(0.75)
	self:linear( Var "LyricDuration" * 0.75)
	if side == "Front" then
		self:cropright(0)
	else
		self:cropleft(1)
	end
	self:sleep( Var "LyricDuration" * 0.25 )
	self:linear(0.2)
	self:diffusealpha(0)
	return self
end

-- formerly in 02 HelpDisplay.lua, although nothing uses it:
function HelpDisplay:setfromsongorcourse()
	local Artists = {}
	local AltArtists = {}

	local Song = GAMESTATE:GetCurrentSong()
	local Trail = GAMESTATE:GetCurrentTrail( GAMESTATE:GetMasterPlayerNumber() )
	if Song then
		table.insert( Artists, Song:GetDisplayArtist() )
		table.insert( AltArtists, Song:GetTranslitArtist() )
	elseif Trail then
		Artists, AltArtists = Trail:GetArtists()
	end

	self:settips( Artists, AltArtists )
	return self
end

-- Play the sound on the given player's side. Must set SupportPan = true
-- on load.
function ActorSound:playforplayer(pn)
	local fBalance = SOUND:GetPlayerBalance(pn)
	self:get():SetProperty("Pan", fBalance)
	self:play()
	return self
end

function PositionPerPlayer(player, p1X, p2X)
	return player == PLAYER_1 and p1X or p2X
end

-- Make graphics their true size at any resolution.
--[[ Note: for screens taller than wide (i.e. phones, sideways displays),
you'll need to get width rather than height (I just don't feel like
uglyfying my code just to handle rare cases). -shake ]]
function GetReal()
	local theme = THEME:GetMetric("Common","ScreenHeight")
	local res = PREFSMAN:GetPreference("DisplayHeight")
	return theme/res
end
function GetRealInverse()
	local theme = THEME:GetMetric("Common","ScreenHeight")
	local res = PREFSMAN:GetPreference("DisplayHeight")
	return res/theme
end

-- command aliases:
function Actor:SetSize(w,h) self:setsize(w,h) return self end

-- Simple draworder mappings
DrawOrder = {
  Background	= -100,
  Underlay		= 0,
  Decorations	= 100,
  Content		= 105,
  Screen		= 120,
  Overlay		= 200
};

-- function Actor:SetDrawOrder

-- deprecated aliases:
function Actor:hidden(bHide)
	lua.ReportScriptError("hidden is deprecated, use visible instead. (used on ".. self:GetName() ..")")
	self:visible(not bHide)
end

-- (c) 2006-2012 Glenn Maynard, the Spinal Shark Collective, et al.
-- All rights reserved.
--
-- Permission is hereby granted, free of charge, to any person obtaining a
-- copy of this software and associated documentation files (the
-- "Software"), to deal in the Software without restriction, including
-- without limitation the rights to use, copy, modify, merge, publish,
-- distribute, and/or sell copies of the Software, and to permit persons to
-- whom the Software is furnished to do so, provided that the above
-- copyright notice(s) and this permission notice appear in all copies of
-- the Software and that both the above copyright notice(s) and this
-- permission notice appear in supporting documentation.
--
-- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
-- OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
-- MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
-- THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
-- INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
-- OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
-- OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
-- OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
-- PERFORMANCE OF THIS SOFTWARE.
