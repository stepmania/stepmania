local Noteskin = ... or {}

--bBlanks: 
Noteskin.bBlanks = {
	--["element"] = true|false;
	["Hold Tail Active"] = true;
	["Roll Tail Active"] = true;
}
Noteskin.ElementRedirs = {
	--["element"] = "redirected_element";
	["Hold Head Active"] = "Tap Note";
	["Hold Head Inactive"] = "Tap Note";
	["Roll Head Active"] = "Tap Note";
	["Roll Head Inactive"] = "Tap Note";
	["Tap Fake"] = "Tap Note";
	--
	["Hold Topcap Inactive"] = "Hold Topcap Active";
	["Hold Body Inactive"] = "Hold Body Active";
	["Hold Bottomcap Inactive"] = "Hold Bottomcap Active";
	["Hold Tail Inactive"] = "Hold Tail Active";
	--
	["Roll Topcap Inactive"] = "Roll Topcap Active";
	["Roll Body Inactive"] = "Roll Body Active";
	["Roll Bottomcap Inactive"] = "Roll Bottomcap Active";
	["Roll Tail Inactive"] = "Roll Tail Active";
}
Noteskin.ButtonRedirs = {
	Center = "Center";
	UpLeft = "UpLeft";
	UpRight = "UpLeft";
	DownLeft = "DownLeft";
	DownRight = "DownLeft";
}
Noteskin.BaseRotX = {
	Center = 0;
	UpLeft = 0;
	UpRight = 0;
	DownLeft = 0;
	DownRight = 0;
}
Noteskin.BaseRotY = {
	Center = 0;
	UpLeft = 0;
	UpRight = 180;
	DownLeft = 0;
	DownRight = 180;
}

local function func()
	local sButton = Var "Button"
	local sElement = Var "Element"
	
	if Noteskin.bBlanks[sElement] then
		return Def.Actor {}
	end
	
	--local ButtonToLoad = Noteskin.ButtonRedirs[sButton]
	local ElementToLoad = Noteskin.ElementRedirs[sElement]
	if not ElementToLoad then
		ElementToLoad = sElement
	end
	
	--update: ahora receptor también
	if sElement == "Explosion"
	or sElement == "Tap Lift"
	or sElement == "Tap Mine"
	or sElement == "Receptor"
	then
		sButton = "UpLeft"
	end
	local path = NOTESKIN:GetPath(Noteskin.ButtonRedirs[sButton],ElementToLoad)
	--Graficos separados para holds y rolls
	if string.find(sElement,"Hold") or string.find(sElement,"Roll") then
		path = NOTESKIN:GetPath(sButton,ElementToLoad)
	end
	
	local t = LoadActor(path)
	--Rotaciones independientes por elemento
	--[[local dRotationX = Noteskin.BaseRotX[sButton][ElementToLoad]
	if not dRotationX then
		dRotationX = Noteskin.BaseRotX[sButton]["Common"]
	end
	
	local dRotationY = Noteskin.BaseRotY[sButton][ElementToLoad]
	if not dRotationY then
		dRotationY = Noteskin.BaseRotY[sButton]["Common"]
	end]]

	t.BaseRotationX=Noteskin.BaseRotX[sButton]
	t.BaseRotationY=Noteskin.BaseRotY[sButton]
	
	return t
end

Noteskin.Load = func
Noteskin.CommonLoad = func

return Noteskin