local ret = ... or {};

ret.RedirTable =
{
	Key1 = "Key",
	Key2 = "Key",
	Key3 = "Key",
	-- should work? doesn't though.
	Key4 = GAMESTATE:IsSideJoined('PlayerNumber_P2') and "Space" or "Key",
	Key5 = "Key",
	Key6 = "Key",
	Key7 = "Key",
};

local OldRedir = ret.Redir;
ret.Redir = function(sButton, sElement)
	sButton, sElement = OldRedir(sButton, sElement);

	-- Instead of separate hold heads, use the tap note graphics.
	if sElement == "Hold Head Inactive" or
	   sElement == "Hold Head Active" or
	   sElement == "Roll Head Inactive" or
	   sElement == "Roll Head Active"
	then
		sElement = "Tap Note";
	end

	sButton = ret.RedirTable[sButton];

	return sButton, sElement;
end

-- To have separate graphics for each hold part:
--[[
local OldRedir = ret.Redir;
ret.Redir = function(sButton, sElement)
	-- Redirect non-hold, non-roll parts.
	if string.find(sElement, "hold") then
		return sButton, sElement;
	end
	return OldRedir(sButton, sElement);
end
]]

local OldFunc = ret.Load;
function ret.Load()
	local t = OldFunc();

	-- The main "Explosion" part just loads other actors; don't rotate
	-- it.  The "Hold Explosion" part should not be rotated.
	if Var "Element" == "Explosion" or
	   Var "Element" == "Roll Explosion" or
	   Var "Element" == "Hold Explosion" then
		t.BaseRotationZ = nil;
	end
	return t;
end

ret.PartsToRotate =
{
	["Go Receptor"] = true,
	["Ready Receptor"] = true,
	["Tap Explosion Bright"] = true,
	["Tap Explosion Dim"] = true,
	["Tap Note"] = true,
	["Hold Head Active"] = true,
	["Hold Head Inactive"] = true,
	["Roll Head Active"] = true,
	["Roll Head Inactive"] = true,
};
ret.Rotate =
{
	Key1 = 0,
	Key2 = 0,
	Key3 = 0,
	Key4 = 0,
	Key5 = 0,
	Key6 = 0,
	Key7 = 0,
};

--
-- If a derived skin wants to have separate UpLeft graphics,
-- use this:
--
-- ret.RedirTable.UpLeft = "UpLeft";
-- ret.RedirTable.UpRight = "UpLeft";
-- ret.Rotate.UpLeft = 0;
-- ret.Rotate.UpRight = 90;
--
ret.Blank =
{
	["Hold Topcap Active"] = true,
	["Hold Topcap Inactive"] = true,
	["Roll Topcap Active"] = true,
	["Roll Topcap Inactive"] = true,
	["Hold Tail Active"] = true,
	["Hold Tail Inactive"] = true,
	["Roll Tail Active"] = true,
	["Roll Tail Inactive"] = true,
};

return ret;
