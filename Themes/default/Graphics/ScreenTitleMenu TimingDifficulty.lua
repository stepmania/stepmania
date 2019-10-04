local icon_params = {
	base_color = color("#981F41"),
	label_text = Screen.String("TimingDifficulty"),
	value_text = GetTimingDifficulty() == 9 and Screen.String("Hardest Timing") or GetTimingDifficulty()
}

return LoadActor(THEME:GetPathG("", "_title_info_icon"), icon_params)
