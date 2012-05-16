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

function split(delimiter, text)
	local list = {}
	local pos = 1
	while 1 do
		local first,last = string.find(text, delimiter, pos)
		if first then
			table.insert(list, string.sub(text, pos, first-1))
			pos = last+1
		else
			table.insert(list, string.sub(text, pos))
			break
		end
	end
	return list
end

function join(delimiter, list)
	local ret = list[1]
	for i = 2,table.getn(list) do
		ret = ret .. delimiter .. list[i]
	end
	return ret or ""
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

function ArgsIfPlayerJoinedOrNil(arg1,arg2)
	if arg1==nil then arg1=arg2
	elseif arg2==nil then arg2=arg1 end
	return (GAMESTATE:IsSideJoined(PLAYER_1) and arg1 or nil),(GAMESTATE:IsSideJoined(PLAYER_2) and arg2 or nil)
end	

function Center1Player()
	local styleType = GAMESTATE:GetCurrentStyle():GetStyleType()
	-- always center in OnePlayerTwoSides.
	if styleType == "StyleType_OnePlayerTwoSides" then
		return true
	-- only Center1P if Pref enabled and OnePlayerOneSide.
	-- (implicitly excludes Rave, Battle, Versus, Routine)
	elseif PREFSMAN:GetPreference("Center1Player") then
		return styleType == "StyleType_OnePlayerOneSide"
	else
		return false
	end
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
	if getmetatable(v) and (getmetatable(v))["__tobool"] and type((getmetatable(v))["__tobool"])=="function" then
		return (getmetatable(v))["__tobool"](v)
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
	end
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

