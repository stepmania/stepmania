-- ProductivityHelpers: A set of useful aliases for theming.
--[[ 3.9 Functions ]]
Game = {
	GetStage = function()
		
	end,
}
--[[ Aliases ]]

-- Blend Modes
-- Aliases for blend modes.
Blend = {
	Normal			= 'BlendMode_Normal',
	Add				= 'BlendMode_Add',
	Subtract		= 'BlendMode_Subtract',
	Modulate		= 'BlendMode_Modulate',
	CopySource		= 'BlendMode_CopySrc',
	AlphaMask		= 'BlendMode_AlphaMask',
	AlphaKnockout	= 'BlendMode_AlphaKnockout',
	AlphaMultiply	= 'BlendMode_AlphaMultiply',
	Multiply		= 'BlendMode_WeightedMultiply',
	Invert			= 'BlendMode_InvertDest',
	NoEffect		= 'BlendMode_NoEffect',
}
function StringToBlend(s) return Blend[s] or nil end

-- EffectMode
-- Aliases for EffectMode (aka shaders)
EffectMode = {
	Normal			= 'EffectMode_Normal',
	Unpremultiply	= 'EffectMode_Unpremultiply',
	ColorBurn		= 'EffectMode_ColorBurn',
	ColorDodge		= 'EffectMode_ColorDodge',
	VividLight		= 'EffectMode_VividLight',
	HardMix			= 'EffectMode_HardMix',
	Overlay			= 'EffectMode_Overlay',
	Screen			= 'EffectMode_Screen',
	YUYV422			= 'EffectMode_YUYV422',
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

--[[ Actor commands ]]

function PositionPerPlayer(player, p1X, p2X)
	return player == PLAYER_1 and p1X or p2X
end

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

function pname(pn) return ToEnumShortString(pn) end

function math.round(num, pre)
	if pre and pre < 0 then pre = 0 end
	local mult = 10^(pre or 0) 
	if num >= 0 then return math.floor(num*mult+.5)/mult 
	else return math.ceil(num*mult-.5)/mult end
end

--[[ end helper functions ]]
-- this code is in the public domain.
