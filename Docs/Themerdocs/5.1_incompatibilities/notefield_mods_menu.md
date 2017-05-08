# Purpose

This is a generalized system for creating mods that use the 5.1 mod system
and can be picked in a nesty menu to apply in gameplay.

This is not a system for changing the mods affecting a player as part of a
scripted simfile.

Mods chosen can be saved to the player's profile, and will be loaded when the
profile is loaded.

Reloading scripts will clear the mod choices for the players.  Mods saved to
profile will not be reloaded when scripts are reloaded.


# Outline

The notefield_mods_actor must be in ScreenGameplay overlay to apply the mods.
```lua
t[#t+1]= notefield_mods_actor()
```

get_notefield_mods_toggle_menu and get_notefield_mods_value_menu are two
different ways of presenting the menu of choices.

add_notefield_menu_choice can be called to add a custom choice to the menu.

get_notefield_mods_with_player_options returns a string of translated mods
currently applied for a player, and also PlayerOptions fields that do not
affect the notefield (such as turn mods, drain setting, and so forth).
ScreenGameplay and ScreenEvaluation call it to set the PlayerOptions text
they display, so you probably don't need to.


# Details

## Putting the mods menu in ScreenNestyPlayerOptions
Example:
```lua
get_notefield_mods_toggle_menu(true, true),
```
Explanation:
```lua
get_notefield_mods_toggle_menu(with_save, no_sections)
```
get_notefield_mods_toggle_menu returns a submenu with options for all the
mods and options for saving them to the profile.

If with_save is false, the options for saving the mods to the profile will
not be in the menu.

If no_sections is false, the options will be split up into submenus.

### Alternative
```lua
get_notefield_mods_value_menu(true, true),
```
Same args as the toggle menu function.  Each choice is a settable number
instead of a toggleable bool.


## Making mods take effect in gameplay
Example:
```lua
t[#t+1]= notefield_mods_actor()
```
This function returns an actor that does the stuff with the thing at the
time.


## Showing mod string
ScreenGameplay and ScreenEvaluation already call this function, so you
probably don't need to.

Example:
```lua
local mod_str= get_notefield_mods_with_player_options(PLAYER_1, false)
```
Explanation:
```lua
get_notefield_mods_with_player_options(pn, hide_fail)
```
Returns a string of comma serpentine modifiers with percentages.  
pn is the PlayerNumber.  If hide_fail is true, the fail type will not be in
the result.


## Adding a choice to the menu
Example:
A menu choice named "sloshed" that turns on beat and drunk.
```lua
add_notefield_menu_choice("effect", "sloshed", {{"effect.drunk", 1}, {"effect.beat", 1}})
```
Explanation:
```lua
add_notefield_menu_choice(section, name, choice)
```
section is the name of the section of the menu the choice belongs in.  
name is the name of the choice.  
choice is a table of mod and magnitude pairs.  When the player picks the
choice, each mod in the list will be set to the magnitude paired with it.

If the value style menu is choosed, then the magnitude for each mod is
multiplied by the value set by the player.


## Adding a mod that can be used in choices
Example:
```lua
add_notefield_mod("candy", "pocky", function(mag, field)
  field:all_columns_mod("get_note_zoom_x", {name= "pocky", sum_type= "*",
    {"max", 0, {"*", mag, {"sin", {"/", "y_offset", 30}}}}})
  end)
```
Explanation:
```lua
add_notefield_mod(section, name, mod_func)
```
The mod_func function will be passed the magnitude the player set for the
modifier, and the player's notefield.  Anything is fair game from there.
