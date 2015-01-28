local ret = ... or {};

local pn = Var "Player"

local OldRedir = ret.Redir;
ret.Redir = function(sButton, sElement)
	-- Replace all receptors with genericized ones
	if string.find(sButton,"RightFoot") and string.find(sElement, "Receptor") then
		return "AnyRightFoot", "Receptor"
	end
	if string.find(sButton,"LeftFoot") and string.find(sElement, "Receptor") then
		return "AnyLeftFoot", "Receptor"
	end
	if string.find(sButton,"RightFist") and string.find(sElement, "Receptor") then
		return "AnyRightFist", "Receptor"
	end
	if string.find(sButton,"LeftFist") and string.find(sElement, "Receptor") then
		return "AnyLeftFist", "Receptor"
	end

	-- Replace all Lifts with genericized ones
	if string.find(sButton,"RightFoot") and string.find(sElement, "Tap Lift") then
		return "AnyRightFoot", "Tap Lift"
	end
	if string.find(sButton,"LeftFoot") and string.find(sElement, "Tap Lift") then
		return "AnyLeftFoot", "Tap Lift"
	end
	if string.find(sButton,"RightFist") and string.find(sElement, "Tap Lift") then
		return "AnyRightFist", "Tap Lift"
	end
	if string.find(sButton,"LeftFist") and string.find(sElement, "Tap Lift") then
		return "AnyLeftFist", "Tap Lift"
	end

	-- Replace all explosions with AnyRight* or AnyLeft*. Is there a better way to do this?
	if string.find(sButton,"RightFoot") and string.find(sElement, "Explosion") then
		return "AnyRightFoot", "Explosion"
	end
	if string.find(sButton,"LeftFoot") and string.find(sElement, "Explosion") then
		return "AnyLeftFoot", "Explosion"
	end
	if string.find(sButton,"RightFist") and string.find(sElement, "Explosion") then
		return "AnyRightFist", "Explosion"
	end
	if string.find(sButton,"LeftFist") and string.find(sElement, "Explosion") then
		return "AnyLeftFist", "Explosion"
	end

	if (GAMESTATE:GetCurrentStyle(pn):GetStepsType() == "StepsType_Kickbox_Human") then
		if string.find(sButton, "LeftFoot") then
			sButton = "AnyLeftFoot"
		end
		if string.find(sButton, "RightFoot") then
			sButton = "AnyRightFoot"
		end
		--
		if string.find(sButton, "LeftFist") then
			sButton = "AnyLeftFist"
		end
		if string.find(sButton, "RightFist") then
			sButton = "AnyRightFist"
		end
	end

	if (GAMESTATE:GetCurrentStyle(pn):GetStepsType() == "StepsType_Kickbox_Insect") then
		if string.find(sButton, "LeftFoot") then
			sButton = "AnyLeftFoot"
		end
		if string.find(sButton, "RightFoot") then
			sButton = "AnyRightFoot"
		end
	end
		-- return OldRedir(sButton, sElement);
	return sButton, sElement
end

return ret