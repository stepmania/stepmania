Branch.PlayerOptions= function()
	if SCREENMAN:GetTopScreen():GetGoToOptions() then
		return "ScreenNestyPlayerOptions"
	else
		return "ScreenStageInformation"
	end
end
