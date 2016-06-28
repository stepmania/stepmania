# Purpose

nesty_menus is a lua based menu system.  It is designed to scale to any
number of nested menu options and be extensible to fit custom menu types.


```lua
local menu= setmetatable({}, nesty_menu_stack_mt)
```

```lua
-- Any parameter to create_actors that is not specified will be given a
-- reasonable default value.
menu:create_actors{
	-- Basic positioning params.
	x= _screen.cx, y= _screen.h*.1, width= _screen.w*.25, height= _screen.h*.8,
	-- translation_section is be used by the items to translate names.
	translation_section= "newfield_options",
	-- With more than one display, submenus go on their own display.
	num_displays= 1,
	-- pn is the player number to pass to various functions that need it.
	pn= PLAYER_1,
	-- Vertical spacing between items.
	el_height= 20,
	-- When an action occurs, the sound with the same name will be played.
	-- These are the basic action names.
	-- It is also possible to make a menu item with a custom sound.
	menu_sounds= {
		-- pop is played when exiting a submenu.
		pop= THEME:GetPathS("Common", "Cancel"),
		-- push is played when entering a submenu.
		push= THEME:GetPathS("_common", "row"),
		-- act is for hitting a menu option.
		act= THEME:GetPathS("Common", "value"),
		-- move is for moving the cursor with Select (teleport to top)
		move= THEME:GetPathS("_switch", "down"),
		-- move_up is when the cursor moves up.
		move_up= THEME:GetPathS("_switch", "up"),
		-- move_down is when the cursor moves down.
		move_down= THEME:GetPathS("_switch", "down"),
		-- inc is used by the adjustable_float menu when the value increases.
		inc= THEME:GetPathS("_switch", "up"),
		-- dec is used by the adjustable_float menu when the value decreases.
		dec= THEME:GetPathS("_switch", "down"),
	},
	-- cursor_params is the params table that will be passed to the cursor
	-- object.
	cursor_params= {
		name= "", -- For seeing the name in the actor tree.  Optional.
		-- parts_name is used to find the parts of the cursor in the Graphics
		-- folder.  In this example, "OptionsCursor Middle",
		-- "OptionsCursor Left", and "OptionsCursor Right", are loaded.
		parts_name= "OptionsCursor",
		-- The nesty_menu_stack_mt passes its player number to the cursor.
	},
	-- display_params control how the menu items are displayed.
	display_params= {
		name= "", -- For seeing the name in the actor tree.  Optional.
		el_zoom= 1, -- Zoom factor to apply to each menu item.
		-- If no_heading is true, the menu will not have an actor for displaying
		-- the submenu name at the top.
		no_heading= false,
		-- If no_status is true, the menu will not have an actor for displaying a
		-- current value under the heading.
		no_status= false,
		-- Zoom factor for the heading actor.  Defaults to el_zoom.
		heading_zoom= el_zoom,
		-- Zoom factor for the status actor.  Defaults to el_zoom.
		status_zoom= el_zoom,
		heading_actor= Def.BitmapText{
			Font= "Common Normal", SetCommand= nesty_option_item_default_set_command},
		status_actor= ..., -- Same as heading_actor.
		-- If the heading actor is not the same height as an element, set
		-- heading_height to its height to position the status and menu elements.
		heading_height= el_height,
		status_height= el_height,
		-- item_params are passed to the menu items that are created.
		item_params= {
			text_font= "Common Normal",
			-- With text_width set to .70, the name will take up 70% of the width.
			text_width= .70,
			-- The value image or text will take up 25% of the width.
			value_width= .25,
			-- text_commands is a table with the commands for the text actor.  Use
			-- it to set the font or OnCommand or whatever.
			text_commands= {},
			-- The value part of the item has a text actor and an image.
			value_text_commands= {},
			value_image_commands= {},
			-- type_images is used to find images to display in the value section.
			-- If there is no image for a type, no image will be shown.
			type_images= {
				-- The bool image must have two states.
				bool= THEME:GetPathG("", "menu_icons/bool"),
				-- The choice image must have two states.
				choice= THEME:GetPathG("", "menu_icons/bool"),
				menu= THEME:GetPathG("", "menu_icons/menu"),
				back= nil,
				-- The adjustable_float menu's type is "number", but it's better to
				-- display the value of the number instead.
				number= nil,
				-- action is used by menu items that do things.
				action= nil,
				-- Used by the enum menu.
				enum= nil,
			},
		},
		-- The menu stack sets other display params to position and size it.
	},
}
```

# Menu items

### Action menu item
```lua
{
	-- name is the name of the item.
	name= "foo",
	-- If translatable is true, the name will be used with the menu's
	-- translation_section to translate the name before displaying it.
	translatable= true,
	-- The player number is passed in by the menu stack.
	execute= function(pn) end,
}
```

### Submenu menu item

```lua
nesty_options.submenu("foo", {})
```
First arg is the name of the submenu.  This will be translated.  
Second arg is the table of items in the menu.

## Number menu items

### Preference
```lua
nesty_options.float_pref_val("BGBrightness", -2, -1, 0, 0, 1, 1)
```
* Preference name
* Min scale  
	The smallest step the number can be adjusted by.
* Starting scale  
	Starting scale is ignored if the difference between min scale and max scale
	is less than 4.
* Max scale  
	The largest step the number can be adjusted by.
	Scale values are in terms of 10^n.  -2 means the value can be adjusted in
	steps of 0.01.
* Min value  
	If min value is nil, no minimum is enforced.
* Max value  
	If max value is nil, no maximum is enforced.
* Reset value  
	If reset value is nil, the number will be set to 0 when the player chooses
	Reset on the menu.

### Song Modifier
```lua
nesty_options.float_song_mod_val("MusicRate", -2, -1, 0, .5, 2, 1)
```
* SongOptions function name
* Min scale
* Starting scale
* Max scale
* Min value
* Max value
* Reset value

### Player Modifier
```lua
nesty_options.float_player_mod_val("BatteryLives", 0, 0, 0, 1, 10, 4)
```
* PlayerOptions function name
* Min scale
* Starting scale
* Max scale
* Min value
* Max value
* Reset value

### Profile value
```lua
nesty_options.float_profile_val("WeightPounds", 0, 1, 2, 0, nil, 0)
```
* Profile function name
* Min scale
* Starting scale
* Max scale
* Min value
* Max value
* Reset value

### Lua config value
```lua
nesty_options.float_config_val(baz_config, "bazziness", 0, 1, 2, 0, 100, 0)
```
* Config object
* Value name
* Min scale
* Starting scale
* Max scale
* Min value
* Max value
* Reset value

## Bool menu items

### Song Modifier
```lua
nesty_options.bool_song_mod_val("AssistClap")
```
* SongOptions function name

### Player Modifier
```lua
nesty_options.player_mod_val("NoHolds")
```
* PlayerOptions function name

### Profile bool
```lua
nesty_options.bool_profile_val("IsMale")
```
* Profile function name

### Config bool
```lua
nesty_options.bool_config_val(baz_config, "baz_away")
```
* Config object
* Field name

## Number with two values
For a number that you want to switch between two values, instead of any
value in a range.  The type icon is the same as a bool.

### Song Modifier
```lua
nesty_options.float_song_mod_toggle_val("Haste", 1, 0)
```
* SongOptions function name
* On value
* Off value

### Config value
```lua
nesty_options.float_config_toggle_val(baz_config, "bazongas", 2, 5)
```
* Config object
* On value
* Off value

## Choices menus

### Config choices
```lua
nesty_options.choices_config_val(baz_config, "baz_type", {"awkward", "cringey", "shameful"})
```
* Config object
* Field name
* Choices table

### Player enum modifier
```lua
nesty_options.enum_player_mod_val("drain", DrainType, "DrainSetting")
```
* Display name
* Enum table
* PlayerOptions function name

### Player enum modifier, single value
For when you want single values of various enums all in one menu.
```lua
	nesty_options.enum_player_mod_single_val("bar_type", "LifeType_Bar", "LifeSetting"),
```
* Display name
* Enum value
* PlayerOptions function name


# Table of menu items
```lua
{
	nesty_options.submenu("foo", foo_options),
	nesty_options.bool_config_val(baz_config, "bazziness"),
	...
}
```
