local icon_params = {
	base_color = color("#981F41"),
	label_text = Screen.String("LifeDifficulty"),
	value_text = GetLifeDifficulty()
}

return LoadActor(THEME:GetPathG("", "_title_info_icon"), icon_params)