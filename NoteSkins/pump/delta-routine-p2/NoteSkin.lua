local Noteskin = {}

--bBlanks:
Noteskin.bBlanks = {
	--["element"] = true|false;
	["Hold Tail Active"] = true;
	["Hold Tail Active"] = true;
	--
	--["Hold Head Active"] = "Tap Note";
	--["Hold Head Inactive"] = "Tap Note";
	--
	["Roll Tail Inactive"] = true;
	["Roll Tail Inactive"] = true;
}
Noteskin.PartsToRotate = {
	--["elemenu"] = true|false;
	["Roll Head Active"] = false;
	["Roll Head Inactive"] = false;
	["NoteHit"] = true;
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
	["Hold Body Active"] = "Hold Body Active";
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
	UpRight = 180;
	DownLeft = 0;
	DownRight = 180;
}

local function func()
	local sButton = Var "Button"
	local sElement = Var "Element"
	
	if Noteskin.bBlanks[sElement] then
		local t = Def.Actor {};
		if Var "SpriteOnly" then
			t = LoadActor( "_blank" );
		end
		return t
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
	--if string.find(sElement,"Hold") or string.find(sElement,"Roll") then
	if ( string.find(sElement,"Hold") or string.find(sElement,"Roll") ) and not ( string.find(sElement,"Head") or string.find(sElement,"Tail") ) then
	--if ( string.find(sElement,"Hold") or string.find(sElement,"Roll") ) and ( string.find(sElement,"Body") ) then
		path = NOTESKIN:GetPath(sButton,ElementToLoad)
	end
	
	local t = LoadActor(path)
	
	local bRotate = Noteskin.PartsToRotate[ElementToLoad]
	--rotate by default
	if bRotate == nil then bRotate = true end
	if bRotate then
		t.BaseRotationX=Noteskin.BaseRotX[sButton]
		t.BaseRotationY=Noteskin.BaseRotY[sButton]
	end
	
	return t
end

Noteskin.Load = func
Noteskin.CommonLoad = func

return Noteskin