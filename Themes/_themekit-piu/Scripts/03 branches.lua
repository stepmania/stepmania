function GameplayNext()
	--Cuando pierdas
	if STATSMAN:GetCurStageStats():AllFailed() then
		--Pierdes, fuera
		return "ScreenStageBreak"
	--¡Pasaste!
	else
		return "ScreenEvaluationNormal"
	end
end

function EvaluationNext()
	local stagesleft = GAMESTATE:GetSmallestNumStagesLeftForAnyHumanPlayer()
	if (stagesleft <= 0 or STATSMAN:GetBestGrade() >= 6) and not GAMESTATE:IsEventMode() then
		--return "ScreenGameOver"
		return "ScreenProfileSave"
	else
		return "ScreenNextStage"
	end
end

function GameOverNext()
	if GAMESTATE:GetCoinMode() == 'CoinMode_Home'
	then
		return "ScreenInit"
	else
		return "ScreenTitleMenu"
	end
end

function StageBreakNext()
	if GAMESTATE:IsEventMode() then
		return "ScreenSelectMusic"
	else
		--return "ScreenGameOver"
		return "ScreenProfileSave"
	end
end
