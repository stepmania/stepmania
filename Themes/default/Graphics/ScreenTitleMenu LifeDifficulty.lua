local icon_params = {
	base_color = color("#F51319"),
	label_text = Screen.String("LifeDifficulty"),
	value_text = GetLifeDifficulty()
}

return LoadActor(THEME:GetPathG("", "_title_info_icon"), icon_params)