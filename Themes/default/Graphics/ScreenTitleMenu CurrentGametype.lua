local icon_params = {
	base_color = color("#981F41"),
	label_text = Screen.String("CurrentGametype"),
	value_text = GAMESTATE:GetCurrentGame():GetName()
}

return LoadActor(THEME:GetPathG("", "_title_info_icon"), icon_params)