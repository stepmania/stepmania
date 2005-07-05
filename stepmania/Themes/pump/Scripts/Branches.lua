function BranchMusic()
	if PlayModeName() == "Rave" then
		return "ScreenSelectMusic"
	elseif Condition2=PlayModeName() == "Regular" then
		return "ScreenEz2SelectMusic"
	elseif Condition3=PlayModeName() == "Nonstop" then
		return "ScreenSelectCourse"
	elseif Condition4=PlayModeName() == "Oni" then
		return "ScreenSelectCourse"
	elseif Condition5=PlayModeName() == "Endless" then
		return "ScreenSelectCourse"
	elseif Condition6=PlayModeName() == "Battle" then
		return "ScreenSelectMusic"
	else
		return "ScreenEz2SelectMusic"
	end
end

function BranchEvaluation()
	if PlayModeName() == "Rave" then
		return "ScreenEvaluationRave"
	elseif Condition2=PlayModeName() == "Regular" then
		return "ScreenEvaluationStage"
	elseif Condition3=PlayModeName() == "Nonstop" then
		return "ScreenEvaluationNonstop"
	elseif Condition4=PlayModeName() == "Oni" then
		return "ScreenEvaluationOni"
	elseif Condition5=PlayModeName() == "Endless" then
		return "ScreenEvaluationEndless"
	elseif Condition6=PlayModeName() == "Battle" then
		return "ScreenEvaluationBattle"
	else
		return "ScreenEvaluationStage"
	end
end

