local ret = ... or {};

ret.RedirTable =
{
	DownLeft = "DownLeft",
	DownRight = "DownLeft",
	UpLeft = "DownLeft",
	UpRight = "DownLeft",
	Center = "Center",
};

local CommonTable = {
	["Ready Receptor"] = true;
	["Roll Explosion"] = true;
	["Hold Explosion"] = true;
	["Tap Explosion Bright"] = true;
	["Tap Explosion Dim"] = true;
};

ret.Blank =
{
	["Hold Tail Inactive"] = true,
	["Hold Tail Active"] = true,
	["Roll Tail Inactive"] = true,
	["Roll Tail Active"] = true,
};

local OldRedir = ret.Redir;
ret.Redir = function(sButton, sElement)
	sButton, sElement = OldRedir(sButton, sElement);
	
	if CommonTable[sButton] and CommonTable[sElement] then
		return "Common", sElement;
	end

	-- Instead of separate hold heads, use the tap note graphics.
	if sElement == "Hold Head Inactive" or
	   sElement == "Hold Head Active" or
	   sElement == "Roll Head Inactive" or
	   sElement == "Roll Head Active"
	then
		sElement = "Tap Note";
	end

	-- Don't redir BottomCaps.
	if sElement == "Hold Body Inactive" or
	   sElement == "Hold Body Active" or
	   sElement == "Roll Body Inactive" or
	   sElement == "Roll Body Active" or
	   sElement == "Hold Topcap Inactive" or
	   sElement == "Hold Topcap Active" or
	   sElement == "Roll Topcap Inactive" or
	   sElement == "Roll Topcap Active" or
	   sElement == "Hold Bottomcap Inactive" or
	   sElement == "Hold Bottomcap Active" or
	   sElement == "Roll Bottomcap Inactive" or
	   sElement == "Roll Bottomcap Active" or
	   sElement == "Go Receptor" or
	   sElement == "Ready Receptor"
	then
		return sButton, sElement;
	end

	sButton = ret.RedirTable[sButton] or sButton;

	return sButton, sElement;
end

local OldFunc = ret.Load;
function ret.Load()
	local t = OldFunc();

	local sButton = Var "Button";
	local sElement = Var "Element";

	-- The main "Explosion" part just loads other actors; don't rotate
	-- it.  The "Hold Explosion" part should not be rotated.
	if sElement == "Explosion" or
	   sElement == "Receptor" or
	   sElement == "Roll Explosion" or
	   sElement == "Hold Explosion" then
		t.BaseRotationZ = nil;
	end

	if CommonTable[sButton] and CommonTable[sElement] then
		t.BaseRotationZ = CommonTable[Var "Button"];
	end

	return t;
end

ret.PartsToRotate =
{
	["Receptor"] = true,
	["Tap Explosion Bright"] = true,
	["Tap Explosion Dim"] = true,
	["Tap Note"] = true,
	["Hold Head Active"] = true,
	["Hold Head Inactive"] = true,
	["Roll Head Active"] = true,
	["Roll Head Inactive"] = true,
	["Hold Tail Active"] = true,
	["Hold Tail Inactive"] = true,
	["Roll Tail Active"] = true,
	["Roll Tail Inactive"] = true,
};
ret.Rotate =
{
	DownLeft = 0,
	UpLeft = 90,
	UpRight = 180,
	DownRight = 270,
	Center = 0,
};

return ret;
