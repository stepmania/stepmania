-- ProductivityHelpers: A set of useful aliases for theming.
-- This is the sm-ssc version. You should not be using this in themes for
-- SM4 right now... We'll post an updated version soon.

--[[ Globals ]]
function IsArcade()
	local sPayMode = GAMESTATE:GetCoinMode();
	local bIsArcade = (sPayMode ~= 'CoinMode_Home');
	return bIsArcade;
end

function IsHome()
	local sPayMode = GAMESTATE:GetCoinMode();
	local bIsHome = (sPayMode == 'CoinMode_Home');
	return bIsHome;
end

function IsFreePlay()
	if IsArcade() then
		return (GAMESTATE:GetCoinMode() == 'CoinMode_Free');
	else
		return false
	end
end

function Center1Player()
	if GAMESTATE:GetCurrentStyle():GetStyleType() == "StyleType_OnePlayerTwoSides" then
		return true
	elseif PREFSMAN:GetPreference("Center1Player") then
		if GAMESTATE:GetCurrentStyle():GetStyleType() == "StyleType_OnePlayerOneSide" then
			return true
		else
			return false
		end
	else
		return false
	end
--[[ 	return PREFSMAN:GetPreference("Center1Player") and
	THEME:GetMetric("ScreenGameplay","AllowCenter1Player") and 
	not GAMESTATE:GetPlayMode("PlayMode_Battle") and 
	not GAMESTATE:GetPlayMode("PlayMode_Rave") and 
	GAMESTATE:GetCurrentStyle():GetStyleType() == "StyleType_OnePlayerOneSide"; --]]
end

--[[ 3.9 Conditionals ]]
Condition = {
	Hour = function()
		return Hour()
	end,
	IsDemonstration = function()
		return GAMESTATE:IsDemonstration()
	end,
	CurSong = function(sSongName)
		return GAMESTATE:GetCurrentSong():GetDisplayMainTitle() == sSongName
	end,
	DayOfMonth = function()
		return DayOfMonth()
	end,
	MonthOfYear = function()
		return MonthOfYear()
	end,
	UsingModifier = function(pnPlayer, sModifier)
		return GAMESTATE:PlayerIsUsingModifier( pnPlayer, sModifier );
	end,
}
--[[ 3.9 Functions ]]
Game = {
	GetStage = function()
	
	end,
}
--[[ Aliases ]]

-- Blend Modes
-- Aliases for blend modes.
Blend = {
	Normal   = 'BlendMode_Normal',
	Add      = 'BlendMode_Add',
	Modulate = 'BlendMode_Modulate',
	Multiply = 'BlendMode_WeightedMultiply',
	Invert   = 'BlendMode_InvertDest',
	NoEffect = 'BlendMode_NoEffect',
}

-- Health Declarations
-- Used primarily for lifebars.
Health = {
	Max    = 'HealthState_Hot',
	Alive  = 'HealthState_Alive',
	Danger = 'HealthState_Danger',
	Dead   = 'HealthState_Dead'
}

-- Make graphics their true size at any resolution.
--[[
	Note: for screens taller than wide (i.e. phones, sideways displays),
	you'll need to get width rather than height (I just don't feel like
	uglyfying my code just to handle rare cases). -shake
--]]

-- useful
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

function Actor:Real()
	-- scale back down to real pixels.
	self:basezoom(GetReal())
	-- don't make this ugly
	self:SetTextureFiltering(false)
end

-- Scale things back up after they have already been scaled down.
function Actor:RealInverse()
	-- scale back up to theme resolution
	self:basezoom(GetRealInverse())
	self:SetTextureFiltering(true)
end

--[[ Actor commands ]]
function Actor:CenterX()
	self:x(SCREEN_CENTER_X)
end

function Actor:CenterY()
	self:y(SCREEN_CENTER_Y)
end

-- xy(actorX,actorY)
-- Sets the x and y of an actor in one command.
function Actor:xy(actorX,actorY)
	self:x(actorX)
	self:y(actorY)
end

function PositionPerPlayer(player, p1X, p2X)
	return player == PLAYER_1 and p1X or p2X
end

-- MaskSource([clearzbuffer])
-- Sets an actor up as the source for a mask. Clears zBuffer by default.
function Actor:MaskSource(noclear)
	if noclear == true then
		self:clearzbuffer(true)
	end
	self:zwrite(true)
	self:blend('BlendMode_NoEffect')
end

-- MaskDest()
-- Sets an actor up to be masked by anything with MaskSource().
function Actor:MaskDest()
	self:ztest(true)
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
	self:effecmagnitude(1,1.125,1)
end

--[[ BitmapText commands ]]

-- PixelFont()
-- An alias that turns off texture filtering.
-- Named because it works best with pixel fonts.
function BitmapText:PixelFont()
	self:SetTextureFiltering(false)
end

-- Stroke(color)
-- Sets the text's stroke color.
function BitmapText:Stroke(c)
	self:strokecolor( c )
end

-- NoStroke()
-- Removes any stroke.
function BitmapText:NoStroke()
	self:strokecolor( color("0,0,0,0") )
end

-- Set Text With Format (contributed by Daisuke Master)
-- this function is my hero - shake
function BitmapText:settextf(...)
	self:settext(string.format(...))
end

-- DiffuseAndStroke(diffuse,stroke)
-- Set diffuse and stroke at the same time.
function BitmapText:DiffuseAndStroke(diffuseC,strokeC)
	self:diffuse(diffuseC)
	self:strokecolor(strokeC)
end;
--[[ end BitmapText commands ]]

--[[ ----------------------------------------------------------------------- ]]

--[[ profile stuff ]]
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
--[[ end profile stuff]]

--[[ ----------------------------------------------------------------------- ]]

--[[ helper functions ]]
function tobool(v)
	if type(v) == "string" then
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

function pname(pn)
	return ToEnumShortString(pn)
end

function math.round(num, pre)
	if pre and pre < 0 then pre = 0 end
	local mult = 10^(pre or 0) 
	if num >= 0 then return math.floor(num*mult+.5)/mult 
	else return math.ceil(num*mult-.5)/mult end
end

--[[ end helper functions ]]
-- this code is in the public domain.
