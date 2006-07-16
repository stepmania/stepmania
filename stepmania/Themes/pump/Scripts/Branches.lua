function BranchMusic()
	if PlayModeName() == "Rave" then
		return "ScreenSelectMusic"
	elseif PlayModeName() == "Regular" then
		return "ScreenEz2SelectMusic"
	elseif PlayModeName() == "Nonstop" then
		return "ScreenSelectCourse"
	elseif PlayModeName() == "Oni" then
		return "ScreenSelectCourse"
	elseif PlayModeName() == "Endless" then
		return "ScreenSelectCourse"
	elseif PlayModeName() == "Battle" then
		return "ScreenSelectMusic"
	else
		return "ScreenEz2SelectMusic"
	end
end

function BranchEvaluation()
	if PlayModeName() == "Rave" then
		return "ScreenEvaluationRave"
	elseif PlayModeName() == "Regular" then
		return "ScreenEvaluationStage"
	elseif PlayModeName() == "Nonstop" then
		return "ScreenEvaluationNonstop"
	elseif PlayModeName() == "Oni" then
		return "ScreenEvaluationOni"
	elseif PlayModeName() == "Endless" then
		return "ScreenEvaluationEndless"
	elseif PlayModeName() == "Battle" then
		return "ScreenEvaluationBattle"
	else
		return "ScreenEvaluationStage"
	end
end

