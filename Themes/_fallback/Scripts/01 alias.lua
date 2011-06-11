--[[ sm-ssc aliases (non-compatibility)
This is mainly here for making commands case-insensitive without needing to
clutter up the C++ code. It can also be used to add custom functions that
wouldn't otherwise belong somewhere else.
--]]

-- in fact, this probably belongs in Sprite.lua...
function Sprite:cropto(w,h)
	self:CropTo(w,h)
end

function Actor:SetSize(w,h)
	self:setsize(w,h)
end

-- shorthand! this is tedious to type and makes things ugly so let's make it shorter.
-- screen.w, screen.h, etc.
local _screen = {
	w  = SCREEN_WIDTH,
	h  = SCREEN_HEIGHT,
	cx = SCREEN_CENTER_X,
	cy = SCREEN_CENTER_Y
}

if Screen.String then
	ScreenString = Screen.String
end

if Screen.Metric then
	ScreenMetric = Screen.Metric
end