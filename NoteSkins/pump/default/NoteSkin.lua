local Noteskin = {}

--bBlanks:
Noteskin.bBlanks = {
	--["element"] = true|false;
	["Hold Tail Active"] = true;
	["Hold Tail Active"] = true;
	["Roll Tail Inactive"] = true;
	["Roll Tail Inactive"] = true;
}
Noteskin.ElementRedirs = {
	--["element"] = "redirected_element";
	["Hold Head Active"] = "Tap Note";
	["Hold Head Inactive"] = "Tap Note";
	["Roll Head Active"] = "Roll Head Active";
	["Roll Head Inactive"] = "Roll Head Active";
	["Tap Fake"] = "Tap Note";
	--
	["Hold Topcap Inactive"] = "Hold Topcap Active";
	["Hold Body Inactive"] = "Hold Body Active";
	["Hold Bottomcap Inactive"] = "Hold Bottomcap Active";
	["Hold Tail Inactive"] = "Hold Tail Active";
	--
	["Roll Topcap Active"] = "Hold Topcap Active";
	["Roll Body Active"] = "Hold Body Active";
	["Roll Bottomcap Active"] = "Hold Bottomcap Active";
	["Roll Tail Active"] = "Hold Tail Active";
	--
	["Roll Topcap Inactive"] = "Hold Topcap Active";
	["Roll Body Inactive"] = "Hold Body Active";
	["Roll Bottomcap Inactive"] = "Hold Bottomcap Active";
	["Roll Tail Inactive"] = "Hold Tail Active";
}
Noteskin.ButtonRedirs = {
	Center = "Center";
	UpLeft = "UpLeft";
	UpRight = "UpRight";
	DownLeft = "DownLeft";
	DownRight = "DownRight";
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
	UpRight = 0;
	DownLeft = 0;
	DownRight = 0;
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
	--if ( string.find(sElement,"Hold") or string.find(sElement,"Roll") ) and not ( string.find(sElement,"Head") or string.find(sElement,"Tail") ) then
	--if ( string.find(sElement,"Hold") or string.find(sElement,"Roll") ) and ( string.find(sElement,"Body") ) then
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