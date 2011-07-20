--[[
functions for using HSV colors in StepMania.
Code adapted from http://www.cs.rit.edu/~ncs/color/t_convert.html

changelog:

v 1

= v1.1 =
* Hue(color, newHue) changed to wrap around for invalid values.
* Desaturate(color, percent) renamed to Saturation(color, percent).
______________________________________________________________________________
xxx: support HSL<->RGB (which is different) too?
http://en.wikipedia.org/wiki/HSV_color_space#Conversion_from_RGB_to_HSL_or_HSV

HSL treats absolute brightness of a color at 0.5 in L, whereas
HSV treats absolute brightness of a color at 1.0 in V.

Saturation is different too.
]]

-- HasAlpha(c)
function HasAlpha(c)
	if c[4] then
	 	return c[4]
	else
		return 1
	end
end

-- ColorToHSV(c)
-- Takes in a normal color("") and returns a table with the HSV values.
function ColorToHSV(c)
	local r = c[1]
	local g = c[2]
	local b = c[3]
	-- alpha requires error checking sometimes.
	local a = HasAlpha(c)

	local h = 0
	local s = 0
	local v = 0

	local min = math.min( r, g, b )
	local max = math.max( r, g, b )
	v = max

	local delta = max - min

	-- xxx: how do we deal with complete black?
	if min == 0 and max == 0 then
		-- we have complete darkness; make it cheap.
		return {
			Hue = 0,
			Sat = 0,
			Value = 0,
			Alpha = a
		}
	end

	if max ~= 0 then
		s = delta / max -- rofl deltamax :|
	else
		-- r = g = b = 0; s = 0, v is undefined
		s = 0
		h = -1
		return {
			Hue = h,
			Sat = s,
			Value = v,
			Alpha = 1
		}
	end

	if r == max then
		h = ( g - b ) / delta     -- yellow/magenta
	elseif g == max then
		h = 2 + ( b - r ) / delta -- cyan/yellow
	else
		h = 4 + ( r - g ) / delta -- magenta/cyan
	end

	h = h * 60 -- degrees

	if h < 0 then
		h = h + 360
	end

	return {
		Hue = h,
		Sat = s,
		Value = v,
		Alpha = a
	}
end

-- HSVToColor(hsv)
-- Converts a set of HSV values to a color. hsv is a table.
-- See also: HSV(h, s, v)
function HSVToColor(hsv)
	local i
	local f, q, p, t
	local r, g, b
	local h, s, v

	local a

	s = hsv.Sat
	v = hsv.Value

	if hsv.Alpha then
		a = hsv.Alpha
	else
		a = 0
	end

	if s == 0 then
		return { v, v, v, a }
	end

	h = hsv.Hue / 60

	i = math.floor(h)
	f = h - i
	p = v * (1-s)
	q = v * (1-s*f)
	t = v * (1-s*(1-f))

	if i == 0 then
		return { v, t, p, a }
	elseif i == 1 then
		return { q, v, p, a }
	elseif i == 2 then
		return { p, v, t, a }
	elseif i == 3 then
		return { p, q, v, a }
	elseif i == 4 then
		return { t, p, v, a }
	else
		return { v, p, q, a }
	end
end

-- ColorToHex(c)
-- Takes in a normal color("") and returns the hex representation.
-- Adapted from code in LuaBit (http://luaforge.net/projects/bit/),
-- which is MIT licensed and copyright (C) 2006~2007 hanzhao.
function ColorToHex(c)
	local r = c[1]
	local g = c[2]
	local b = c[3]
	local a = HasAlpha(c)

	local function hex(value)
		value = math.ceil(value)

		local hexVals = { 'A', 'B', 'C', 'D', 'E', 'F' }
		local out = ""
		local last = 0

		while(value ~= 0) do
			last = math.mod(value, 16)
			if(last < 10) then
				out = tostring(last) .. out
			else
				out = hexVals[(last-10)+1] .. out
			end
			value = math.floor(value/16)
		end

		if(out == "") then
			return "00"
		end
		
		return string.format( "%02X", tonumber(out,16) )
	end

	local rX = hex( scale(r, 0, 1, 0, 255) )
	local gX = hex( scale(g, 0, 1, 0, 255) )
	local bX = hex( scale(b, 0, 1, 0, 255) )
	local aX = hex( scale(a, 0, 1, 0, 255) )

	return rX .. gX .. bX .. aX
end

function HSVToHex(hsv)
	return ColorToHex( HSVToColor(hsv) )
end

--[[ you should mainly use these functions                                    ]]

-- this is the lazy one; use this when you mean fullly visible
function HSV(h, s, v)
	local t = {
		Hue = h,
		Sat = s,
		Value = v,
		Alpha = 1
	}
	return HSVToColor(t)
end

-- here's the proper one
function HSVA(h, s, v, a)
	local t = {
		Hue = h,
		Sat = s,
		Value = v,
		Alpha = a
	}
	return HSVToColor(t)
end

function Saturation(color,percent)
	local c = ColorToHSV(color)
	-- error checking
	if percent < 0 then
		percent = 0.0
	elseif percent > 1 then
		percent = 1.0
	end
	c.Sat = percent
	return HSVToColor(c)
end

function Brightness(color,percent)
	local c = ColorToHSV(color)
	-- error checking
	if percent < 0 then
		percent = 0.0
	elseif percent > 1 then
		percent = 1.0
	end
	c.Value = percent
	return HSVToColor(c)
end

function Hue(color,newHue)
	local c = ColorToHSV(color)
	-- handle wrapping
	if newHue < 0 then
		newHue = 360 + newHue
	elseif newHue > 360 then
		--newHue = math.mod(newHue, 360); -- ?? untested
		newHue = newHue - 360
	end
	c.Hue = newHue
	return HSVToColor(c)
end;

function Alpha(color,percent)
	local c = ColorToHSV(color);
	-- error checking
	if percent < 0 then
		percent = 0.0;
	elseif percent > 1 then
		percent = 1.0;
	end;
	c.Alpha = percent;
	return HSVToColor(c);
end;