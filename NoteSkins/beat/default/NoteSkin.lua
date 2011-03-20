local ret = ... or {};

ret.RedirTable =
{
	Key1 = "White",
	Key2 = "Blue",
	Key3 = "White",
	Key4 = "Blue",
	Key5 = "White",
	Key6 = "Blue",
	Key7 = "White",
	scratch = "Red",
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

local OldFunc = ret.Load;
function ret.Load()
	local t = OldFunc();

	-- The main "Explosion" part just loads other actors; don't rotate
	-- it.  The "Hold Explosion" part should not be rotated.
	if Var "Element" == "Explosion" or
	   Var "Element" == "Hold Explosion" then
		t.BaseRotationZ = nil;
	end
	return t;
end

ret.PartsToRotate =
{
	["Go Receptor"] = false,
	["Ready Receptor"] = false,
	["Tap Explosion Bright"] = false,
	["Tap Explosion Dim"] = false,
	["Tap Note"] = false,
	["Hold Head Active"] = false,
	["Hold Head Inactive"] = false,
	["Roll Head Active"] = false,
	["Roll Head Inactive"] = false,
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
