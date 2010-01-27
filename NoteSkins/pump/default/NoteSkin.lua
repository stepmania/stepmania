local Noteskin = ... or {}

Noteskin.bBlanks = {
	--["element"] = true|false;
	["Hold Tail Active"] = true;
	["Roll Tail Active"] = true;
}
Noteskin.ElementRedirs = {
	--["element"] = "redirected_element";
	["Hold Head Active"] = "Tap Note";
	["Roll Head Active"] = "Tap Note";
	["Tap Fake"] = "Tap Note";
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
	
	--redir...
	sElement = string.gsub(sElement,"Inactive","Active")
	sElement = string.gsub(sElement,"inactive","active")
	
	if Noteskin.bBlanks[sElement] then
		return Def.Actor {}
	end
	
	--local ButtonToLoad = Noteskin.ButtonRedirs[sButton]
	local ElementToLoad = Noteskin.ElementRedirs[sElement]
	if not ElementToLoad then
		ElementToLoad = sElement
	end
	
	if sElement == "Explosion" or sElement == "Tap Lift" or sElement == "Tap Mine" then
		sButton = "UpLeft"
	end
	local path = NOTESKIN:GetPath(Noteskin.ButtonRedirs[sButton],ElementToLoad)
	--sean holds o rollos pero que no sean cabezas ni colas, para tener gráficos separados por flechita :D
	if ( string.find(sElement,"Hold") or string.find(sElement,"Roll") ) and not ( string.find(sElement,"Head") or string.find(sElement,"Tail") ) then
		path = NOTESKIN:GetPath(sButton,ElementToLoad)
	end
	
	local t = LoadActor(path)
	t.BaseRotationX=Noteskin.BaseRotX[sButton]
	t.BaseRotationY=Noteskin.BaseRotY[sButton]
	
	return t
end

Noteskin.Load = func
Noteskin.CommonLoad = func

return Noteskin