# Basic menu data example

```lua
local menu_data= {
	{"item", "preference", "Center1Player"},
	{"item", "profile", "WeightPounds"},
	{"item", "player_opttion", "LifeSetting"},
	{"item", "song_option", "MusicRate"},
	notefield_prefs_speed_mod_item(),
	notefield_prefs_speed_type_item(),
	adv_notefield_prefs_menu(),
	{"submenu", "theme_config", {
		 {"item", theme_config, "fancy", "number"},
	}},
}
```

# Type hints used by fallback menu items

## Value types

* ms
* percent
* time
* enum
* number
* string
* boolean


## Main types

* number
* bool
* choice
* toggle_number
* name_value_pairs
* submenu


## Sub types

* preference
* profile
* player_option
* song_option
* config
* data
