local player_config_default= {
	ComboUnderField= true,
	FlashyCombo= false,
	GameplayShowStepsDisplay= true,
	GameplayShowScore= true,
	JudgmentUnderField= true,
	Protiming= false,
	ScreenFilter= 0,
}

player_config= create_lua_config{
	name= "player_config", file= "player_config.lua",
	default= player_config_default,
}
add_standard_lua_config_save_load_hooks(player_config)
