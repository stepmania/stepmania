-- Find a key in tab with the given value.
function FindValue(tab, value)
	for key, name in tab do
		if value == name then
			return key
		end
	end

	return nil
end

-- Return the index of a true value in list.
function FindSelection(list)
	for index, on in list do
		if on then
			return index
		end
	end
	return nil
end

-- Look up each value in a table, returning a table with the resulting strings.
function TableStringLookup(t, group)
	local ret = { }
	for key, val in t do
		Trace(val)
		ret[key] = THEME:GetString(group,val)
	end
	return ret
end

function wrap(val,n)
	local x = val
	Trace( "wrap "..x.." "..n )
	if x<0 then 
		x = x + (math.ceil(-x/n)+1)*n
	end
	Trace( "adjusted "..x )
	local ret = math.mod(x,n)
	Trace( "ret "..ret )
	return ret
end

function fapproach(val, other_val, to_move)
	-- This does not use the (faster) C++ side version of approach because I
	-- don't want to find out how many themes pass a negative speed. -Kyz
	if val == other_val then
		return val -- already done!
	end

	local delta = other_val - val
	local sign = delta / math.abs(delta)
	local toMove = sign*to_move

	if math.abs(toMove) > math.abs(delta) then
		toMove = delta	-- snap
	end

	val = val + toMove
	return val
end

function tableshuffle(t)
	local ret = {}
	for i=1,table.getn(t) do
		table.insert(ret, math.random(i), t[i])
	end
	return ret
end
table.shuffle = tableshuffle

function tableslice(t, num)
	local ret = {}
	for i=1,table.getn(t) do
		table.insert(ret, i, t[i])
	end
	return ret
end
table.slice = tableslice

-- add together the contents of a table
function table.sum(t)
	local sum = 0
	for i=1,#t do
		sum = sum + t[i]
	end
	return sum
end

-- average of all table values
function table.average(t)
	return table.sum(t) / #t
end

-- furthest value from the average of a given table
function table.deviation(t)
	local offset = math.abs(table.average(t))
	for i=1,#t do
		offset = math.max(math.abs(t[i]), offset)
	end
	return offset
end
-- See if this exists
function table.search(t, sFind)
	for _, v in pairs(t) do
		if v == sFind then
			return true
		end
	end
	return false
end
-- Retreive the entry that has this
function table.find(t, sFind)
	for _, v in pairs(t) do
		if v == sFind then
			return _
		end
	end
	return nil
end

--Get the count of all items in a table
function table.itemcount(t)
	local i = 0
	while next(t)~=nil do
		i=i+1
	end
	return i
end

function math.round(num, pre)
	if pre and pre < 0 then pre = 0 end
	local mult = 10^(pre or 0) 
	if num >= 0 then return math.floor(num*mult+.5)/mult 
	else return math.ceil(num*mult-.5)/mult end
end
-- deprecated?
function round(val, decimal)
	if decimal then
		return math.floor((val * 10^decimal) + 0.5) / (10^decimal)
	else
		return math.floor(val+0.5)
	end
end

function GetRandomSongBackground()
	for i=0,50 do
		local song = SONGMAN:GetRandomSong()
		if song then
			local path = song:GetBackgroundPath()
			if path then
				return path
			end
		end
	end
	return THEME:GetPathG("", "_blank")
end

function GetSongBackground()
	local song = GAMESTATE:GetCurrentSong()
	if song then
		local path = song:GetBackgroundPath()
		if path then
			return path
		end
	end
	return THEME:GetPathG("Common","fallback background")
end

function StepsOrTrailToCustomDifficulty(stepsOrTrail)
	if lua.CheckType("Steps", stepsOrTrail) then
		return StepsToCustomDifficulty(stepsOrTrail)
	end
	if lua.CheckType("Trail", stepsOrTrail) then
		return TrailToCustomDifficulty(stepsOrTrail)
	end
end

function IsArcade()
	return GAMESTATE:GetCoinMode() ~= 'CoinMode_Home'
end

function IsHome()
	return GAMESTATE:GetCoinMode() == 'CoinMode_Home'
end

function IsFreePlay()
	return IsArcade() and (GAMESTATE:GetCoinMode() == 'CoinMode_Free') or false
end

function IsCourse()
	local pm = GAMESTATE:GetPlayMode();
	return pm == "PlayMode_Nonstop" or "PlayMode_Oni" or "PlayMode_Endless"
end

function ArgsIfPlayerJoinedOrNil(arg1,arg2)
	if arg1==nil then arg1=arg2
	elseif arg2==nil then arg2=arg1 end
	return (GAMESTATE:IsSideJoined(PLAYER_1) and arg1 or nil),(GAMESTATE:IsSideJoined(PLAYER_2) and arg2 or nil)
end	

function Center1Player()
	local styleType = GAMESTATE:GetCurrentStyle():GetStyleType()
	-- always center in OnePlayerTwoSides ( Doubles ) or TwoPlayersSharedSides ( Couples )
	if styleType == "StyleType_OnePlayerTwoSides" or styleType == "StyleType_TwoPlayersSharedSides" then
		return true
	-- only Center1P if Pref enabled and OnePlayerOneSide.
	-- (implicitly excludes Rave, Battle, Versus, Routine)
	elseif PREFSMAN:GetPreference("Center1Player") then
		return styleType == "StyleType_OnePlayerOneSide"
	else
		return false
	end
end

function IsRoutine()
	local style= GAMESTATE:GetCurrentStyle()
	if style and style:GetStyleType() == "StyleType_TwoPlayersSharedSides" then
		return true
	end
	return false
end

Date = {
	Today = function()
		return string.format("%i%02i%02i", Year(), (MonthOfYear()+1), DayOfMonth())
	end
}

Time = {
	Now = function()
		return string.format( "%02i:%02i:%02i", Hour(), Minute(), Second() )
	end
}

-- file utilities
File = {
	Write = function(path,buf)
		local f = RageFileUtil.CreateRageFile()
		if f:Open(path, 2) then
			f:Write( tostring(buf) )
			f:destroy()
			return true
		else
			Trace( "[FileUtils] Error writing to ".. path ..": ".. f:GetError() )
			f:ClearError()
			f:destroy()
			return false
		end
	end,
	Read = function(path)
		local f = RageFileUtil.CreateRageFile()
		local ret = ""
		if f:Open(path, 1) then
			ret = tostring( f:Read() )
			f:destroy()
			return ret
		else
			Trace( "[FileUtils] Error reading from ".. path ..": ".. f:GetError() )
			f:ClearError()
			f:destroy()
			return nil
		end
	end
}

envTable = GAMESTATE:Env()

-- setenv(name,value)
-- Sets aside an entry for <name> and puts <value> into it.
-- Use a table as <value> to store multiple values
function setenv(name,value) envTable[name] = value end

-- getenv(name)
-- This will return whatever value is at envTable[name].
function getenv(name) return envTable[name] end

-- tobool(v)
-- Converts v to a boolean.
function tobool(v)
	local meta= getmetatable(v)
	if meta and type(meta.__tobool) == "function" then
		return meta.__tobool(v)
	elseif type(v) == "string" then
		local cmp = string.lower(v)
		if cmp == "true" or cmp == "t" then
			return true
		elseif cmp == "false" or cmp == "f" then
			return false
		end
	elseif type(v) == "number" then
		if v == 0 then
			return false
		else
			return true
		end
	elseif type(v) == "boolean" then
		return v
	end
	return nil
end

-- GetPlayerOrMachineProfile(pn)
-- This returns a profile, preferably a player one.
-- If there isn't one, we fall back on the machine profile.
function GetPlayerOrMachineProfile(pn)
	if PROFILEMAN:IsPersistentProfile(pn) then
		-- player profile
		return PROFILEMAN:GetProfile(pn);
	else
		-- machine profile
		return PROFILEMAN:GetMachineProfile();
	end;
end;

function pname(pn) return ToEnumShortString(pn) end

function ThemeManager:GetAbsolutePath(sPath)
	sFinPath = "/Themes/"..self:GetCurThemeName().."/"..sPath
	assert(RageFileManager.DoesFileExist(sFinPath), "the theme element "..sPath.." is missing")
	return sFinPath
end

-- supported aspect ratios
AspectRatios = {
	ThreeFour   = 0.75,		-- (576x760 at 1024x768; meant for rotated monitors?)
	OneOne      = 1.0,		-- 480x480  (uses Y value of specified resolution)
	FiveFour    = 1.25,		-- 600x480 (1280x1024 is a real use case)
	FourThree   = 1.33333,	-- 640x480 (common)
	SixteenTen  = 1.6,		-- 720x480 (common)
	SixteenNine = 1.77778,	-- 853x480 (common)
	EightThree  = 2.66666	-- 1280x480 (two monitors)
}

function WideScale(AR4_3, AR16_9)
	return scale( SCREEN_WIDTH, 640, 854, AR4_3, AR16_9 )
end

local function round(num, idp)
	if idp and idp > 0 then
		local mult = 10 ^ idp;
		return math.floor(num * mult + 0.5) / mult;
	end;
	return math.floor(num + 0.5);
end

function IsUsingWideScreen()
	local curAspect = round(GetScreenAspectRatio(),5);
	for k,v in pairs(AspectRatios) do
		if AspectRatios[k] == curAspect then
			if k == "SixteenNine" or k == "SixteenTen" then
				return true;
			else return false;
			end;
		end;
	end;
end;

-- Usage:  Pass in an ActorFrame and a string to put in front of every line.
-- indent will be appended to at each level of the recursion, to indent each
-- generation further.

function rec_print_children(parent, indent)
	if not indent then indent= "" end
	if #parent > 0 and type(parent) == "table" then
		for i, c in ipairs(parent) do
			rec_print_children(c, indent .. i .. "->")
		end
	elseif parent.GetChildren then
		local pname= (parent.GetName and parent:GetName()) or ""
		local children= parent:GetChildren()
		Trace(indent .. pname .. " children:")
		for k, v in pairs(children) do
			if #v > 0 then
				Trace(indent .. pname .. "->" .. k .. " shared name:")
				rec_print_children(v, indent .. pname .. "->")
				Trace(indent .. pname .. "->" .. k .. " shared name over.")
			else
				rec_print_children(v, indent .. pname .. "->")
			end
		end
		Trace(indent .. pname .. " children over.")
	else
		local pname= (parent.GetName and parent:GetName()) or ""
		Trace(indent .. pname .. "(" .. tostring(parent) .. ")")
	end
end

-- Usage:  Pass in a table and a string to indent each line with.
-- indent will be appended to at each level of the recursion, to indent each
-- generation further.
-- DO NOT pass in a table that contains a reference loop.
-- A reference loop is a case where a table contains a member that is a
-- reference to itself, or contains a table that contains a reference to
-- itself.
-- Short reference loop example:  a= {}   a[1]= a
-- Longer reference loop example:  a= {b= {c= {}}}   a.b.c[1]= a
function rec_print_table(t, indent, depth_remaining)
	if not indent then indent= "" end
	if type(t) ~= "table" then
		Trace(indent .. "rec_print_table passed a " .. type(t))
		return
	end
	depth_remaining= depth_remaining or -1
	if depth_remaining == 0 then return end
	for k, v in pairs(t) do
		if type(v) == "table" then
			Trace(indent .. k .. ": table")
			rec_print_table(v, indent .. "  ", depth_remaining - 1)
		else
			Trace(indent .. "(" .. type(k) .. ")" .. k .. ": " ..
							"(" .. type(v) .. ")" .. tostring(v))
		end
	end
	Trace(indent .. "end")
end

-- Minor text formatting functions from Kyzentun.
-- TODO:  Figure out why BitmapText:maxwidth doesn't do what I want.
-- Intentionally undocumented because they should be moved to BitmapText ASAP
function width_limit_text(text, limit, natural_zoom)
	natural_zoom= natural_zoom or 1
	if text:GetWidth() * natural_zoom > limit then
		text:zoomx(limit / text:GetWidth())
	else
		text:zoomx(natural_zoom)
	end
end

function width_clip_text(text, limit)
	local full_text= text:GetText()
	local fits= text:GetZoomedWidth() <= limit
	local prev_max= #full_text - 1
	local prev_min= 0
	if not fits then
		while prev_max - prev_min > 1 do
			local new_max= math.round((prev_max + prev_min) / 2)
			text:settext(full_text:sub(1, 1+new_max))
			if text:GetZoomedWidth() <= limit then
				prev_min= new_max
			else
				prev_max= new_max
			end
		end
		text:settext(full_text:sub(1, 1+prev_min))
	end
end

function width_clip_limit_text(text, limit, natural_zoom)
	natural_zoom= natural_zoom or text:GetZoomY()
	local text_width= text:GetWidth() * natural_zoom
	if text_width > limit * 2 then
		text:zoomx(natural_zoom * .5)
		width_clip_text(text, limit)
	else
		width_limit_text(text, limit, natural_zoom)
	end
end

function convert_text_to_indented_lines(text, indent, width, text_zoom)
	local text_as_lines= split("\n", text:GetText())
	local indented_lines= {}
	for i, line in ipairs(text_as_lines) do
		local remain= line
		local sub_lines= 0
		repeat
			text:settext(remain)
			local clipped= false
			local indent_mult= 0
			if i > 1 then
				indent_mult= indent_mult + 1
			end
			if sub_lines > 0 then
				indent_mult= 2
			end
			-- On larger resolutions, the font can be squished a bit to fit more on a line.
			local resolution_width_mult= math.max(1, DISPLAY:GetDisplayHeight() / 720)
			local usable_width= (width - (indent * indent_mult)) * resolution_width_mult
			if text:GetWidth() * text_zoom > usable_width then
				clipped= true
				width_clip_text(text, usable_width)
			end
			indented_lines[#indented_lines+1]= {
				indent_mult, text:GetText()}
			remain= remain:sub(#text:GetText()+1)
			sub_lines= sub_lines + 1
		until not clipped
	end
	return indented_lines
end

-- (c) 2005-2011 Glenn Maynard, Chris Danford, SSC
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

