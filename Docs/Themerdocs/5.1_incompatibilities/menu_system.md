# Menu Actors

## Basic

Copy Graphics/generic_menu.lua to your theme and fill in the actor commands
and change the actors however you want.

## Click area

The click_area functions are just wrappers around self:set_clickable_area to
make setting simple rectangles easier.

* click_area.wh(self, w, h)
Top left corner is -w,-h bottom right is w,h.
* click_area.owh(self, w, h)
0,-h to w,h
* click_area.woh(self, w, h)
-w,0 to w,h
* click_area.owoh(self, w, h)
0,0 to w,h
* click_area.lwh(self, l, w, h)
l,-h to l+w,h
* click_area.ltwh(self, l, t, w, h)
l,t to l+w,t+h

self:clickable_area takes a table of points.  Each point is an x and a y.
```lua
self:set_clickable_area{{-w, -h}, {w, -h}, {w, h}, {-w, h}}
```


## Actor Hierarchy

The menu system uses these actor names to find the actors.

menu, display, and item actors must be ActorFrames, because they contain
subactors.  All others can be whatever actor you want to use.

> menu
|> cursor
|> display
||> mouse_scroll_area
||> item
|||> name
|||> value
|||> adjust_up
|||> adjust_down
||> item
||> ...
|> display
|> ...

Half the items in each display are used to make scrolling between items look
nice.  So if there are 8 items in a display, only 4 will be displayed.  Same
goes for displays in the menu.

## Actor Details

All actions and info are carried out through commands on the actors.

### Menu

#### click_area
```lua
click_area.owoh(self, display_width * num_displays, num_items * item_spacing)
```
If the mouse is inside this area, the menu has focus.

#### PlayerizeCommand
```lua
	PlayerizeCommand= function(self, pn)
		self:GetChild("cursor"):diffuse(ColorLightTone(PlayerColor(pn)))
	end,
```
Do something to make the menu look different for each player.

pn will be nil for menus like the edit mode menus.

#### OpenMenuCommand
```lua
	OpenMenuCommand= function(self)
		self:visible(true)
	end,
```

#### CloseMenuCommand
```lua
	CloseMenuCommand= function(self)
		self:visible(false)
	end,
```

#### ShowCommand
```lua
	ShowCommand= function(self)
		self:visible(true)
	end,
```

#### HideCommand
```lua
	HideCommand= function(self)
		self:visible(false)
	end,
```

### Cursor

#### MoveCommand
```lua
		MoveCommand= function(self, params)
			self:stoptweening():linear(.1)
				:xy(params.x + (item_width * .5), params.y)
		end,
```
params.x and params.y are the coordinates of the menu item to move to,
relative to the menu.  params.actor is the menu item actor.

#### NormalMode and AdjustMode

AdjustMode and NormalMode are for two_direction input mode (left, right,
and start buttons only).
So adjust mode means that left and right will change the value instead of
moving the cursor.

If you don't care about two_direction mode or don't need to cue that the mode
has changed, you can ignore it.

```lua
		NormalModeCommand= function(self)
			self:linear(.1):rotationz(0)
		end,
		AdjustModeCommand= function(self)
			self:linear(.1):rotationz(90)
		end,
```


### Display

#### click_area
```lua
click_area.ltwh(self, -display_pad, item_spacing * -.5, display_width, num_items * item_spacing)
```
If the mouse is inside this area, this display has focus.

#### mouse_scroll_area
```lua
click_area.ltwh(self, 0, item_spacing * -.5, name_width, num_items * item_spacing)
```
If the mouse is in this area, and the player uses the scroll wheel, the items
will scroll.  The scroll wheel is also used to change the value of an item,
so the location matters.

#### PlayerizeCommand
Same as for the menu.

#### ActiveCommand and InactiveCommand

Active displays are visible and the player can interact with them.

Inactive displays are hidden.  When the player moves to or from a submenu and
the displays need to be scrolled, inactive displays are given submenus and
scroll on, while the active displays scroll off.

#### ScrollCommand
```lua
		ScrollCommand= function(self, params)
```
params.from and params.to are the position ids to scroll from and to.  The
range from 0 to params.num_items-1 is the visible range, outside that the
actor should not be visible.

params.scroll_type can be "first", "on", "off", or "normal".

"first" is used when a submenu opens and the items are set.

"on" is when an item that was inactive scrolls on from the top or bottom.

"off" is when an item that was active scrolls off the top or bottom.

"normal" is when an item moves without leaving the visible area.

#### GainFocusCommand and LoseFocusCommand

Used when the mouse or cursor enters or leaves the display.

#### OpenSubmenuCommand
```lua
		OpenSubmenuCommand= function(self, info)
```

Used when a submenu opens.  info is the menu data for the submenu.

#### RefreshSubmenuCommand

Similar to OpenSubmenuCommand.  Used when the submenu's items are replaced by
new items.

#### CloseSubmenuCommand

Runs when a submenu closes.



### Item

#### click_area
```lua
click_area.owh(self, item_width, adjuster_size)
```

When the mouse enters this area, the item gains focus.

#### PlayerizeCommand
Same as for display and menu.

#### SetItemCommand
```lua
SetItemCommand= function(self, info)
```
Item was inactive, and will now be displaying something.

The parts of item will also be set individually, so SetItemCommand can be
blank.

info is the item info from the menu data.

#### RefreshItemCommand
Similar to SetItemCommand.  The menu item being displayed has changed.  Can
also be blank.

#### ClearItemCommand
No menu data for item to show.

#### ResetCommand
When the value is reset.

#### ActiveCommand and InactiveCommand

Active items are visible and the player can interact with them.

Inactive items are hidden.  When the player scrolls in a submenu, inactive
items scroll on the top or bottom and active items scroll off.

#### ScrollCommand
```lua
		ScrollCommand= function(self, params)
```
params.from and params.to are the position ids to scroll from and to.  The
range from 0 to params.num_items-1 is the visible range, outside that the
actor should not be visible.

params.scroll_type can be "first", "on", "off", or "normal".

"first" is used when a submenu opens and the items are set.

"on" is when an item that was inactive scrolls on from the top or bottom.

"off" is when an item that was active scrolls off the top or bottom.

"normal" is when an item moves without leaving the visible area.

#### GainFocusCommand and LoseFocusCommand

Used when the mouse or cursor enters or leaves the item.


### Item Parts

#### name

##### click_area
```lua
local zoom= self:GetZoom()
click_area.lwh(self, -self:GetWidth() / 2, name_width/zoom, item_spacing * .5)
```
The player can click the name of an item to interact with it.

##### SetNameCommand
```lua
SetNameCommand= function(self, name)
```

name comes from the menu data.  Menu items from the engine will always have a
string for the name.

#### ClickCommand
React to being clicked by the player.  Also called when the player presses
Start on the item.

#### value

##### SetValueCommand

```lua
SetValueCommand= function(self, info)
```

info comes from the menu data.  The engine menu items use a table with
"{type, value}".  The type is always a string.

Value types used by engine:

* ms
Milliseconds.
* percent
* number
* time
Value is a number of seconds.
* string
Value is a string, already translated.
* boolean
Value is true or false.

The example menu item uses the value type to know when to display a bool as
the on/off image, or a number as a number.

#### adjust_up and adjust_down

##### click_area
For clicking the adjuster.

##### ClickCommand
Also used when the player changes the value with menu buttons.

##### ShowCommand and HideCommand
Not all menu items have an adjust function, so Show and Hide are used to hide
the adjusters when they aren't needed.



# Menu on a screen

## Basic

nesty_menus.make_menu_actors takes care of creating menu controllers, input
and update functions, and loading sounds.

```lua
local menu_layer, menu_controllers= nesty_menus.make_menu_actors{
	actors= menu_actors,
	data= menu_data,
	input_mode= "four_direction",
	repeats_to_big= 10,
	select_goes_to_top= true,
	dont_open= false,
	item_change_callback= function(item, pn) end,
	exit_callback= function(pn) end,
	translation_section= "notefield_options",
	with_click_debug= true,
}
```
menu_layer must be added to the screen.  If its position is not 0,0, the
outlines from with_click_debug will *not* be in the right place.

menu_controllers is the controllers for the menus, indexed by player number.

actors must be the actors returned by using LoadActor on generic_menu.lua,
indexed by player number.
```lua
menu_actors[pn]= LoadActor(THEME:GetPathG("", "generic_menu.lua"), 1, 352, menu_height, 1, menu_x[pn]-(menu_width/2), 138, 36)
```

data is the top level submenu to display.

input_mode can be "two_direction", "two_direction_with_select", or
"four_direction".

repeats_to_big defaults to 10.  If the player holds Left or Right when
adjusting a value until it repeats this many times, it'll adjust by a big
amount instead (usually 10x the small amount).  -1 will just make it always
big.

select_goes_to_top defaults to true.  If it is false, then in four_direction
mode select jumps to the bottom instead.

If dont_open is true, then the menu won't immediately open when the screen
starts.

item_change_callback is called when focus moves to a new menu item.

exit_callback runs when a player closes their toplevel submenu.

translation_section sets the section used to translate all item names and
values.

If with_click_debug is true, then rectangles will be drawn around the
clickable areas of the menu actors, to help tuning them.


### Input modes
(the mouse works the same in all modes)

In two_direction mode, the player only has Left, Right, and Start buttons.
When the player presses Start on an adjustable number, the menu switches to
adjust mode.  Left and Right change the value instead of moving the cursor.

two_direction_with_select means the player also has a Select button.
Start moves the cursor down, Select moves it up.  Left and Right change the
value or open submenus.

With four_direction mode, the player has Left, Right, Up, Down, Start, and
maybe Select.
Up and Down move the cursor.
Left and Right change the value or open submenus.
Start opens submenus.
Select jumps to the top of the current submenu.

Other buttons:
Page up and page down scroll a page of items at once.  Home and End jump to
the top and bottom of the submenu.



# Menu data

Menu data is a series of nested tables.  Each menu item is a table that tells
the system what to do when the player does something.

## Menu Item types

### submenu
```lua
{"submenu", "chart_mods", chart_mods},
```
Second element of the table is the name of the submenu.  The name will be
translated before it is displayed.

Third element is a table of menu items in the submenu.

### action
```lua
{"action", "play_song", change_ready}
```
Third element is the function to call when the player presses Start on the
item.

### close
```lua
{"close", "go_back"}
```
Close items are automatically inserted at the top of every submenu, so you
don't need to manually put one in every submenu.

### item
```lua
{"item", "song_option", "MusicRate"}
```
Second element is the category of item.  Third, fourth, and fifth are
parameters that are different for each category.

#### Preference items
```lua
{"item", "preference", "BGBrightness"}
```
Third element is the name of the preference.  This must match the name of a
preference.

#### Profile items
```lua
{"item", "profile", "WeightPounds"}
```
Only WeightPounds, Voomax, BirthYear, IgnoreStepCountCalories, IsMale,
GoalType, GoalCalories, and GoalSeconds work, because those are the only ones
with Get and Set functions.

#### Song Option items
```lua
{"item", "song_option", "MusicRate"}
```
Third element is the name of the song option.  This must match the name of a
SongOptions function.

#### Player Option items
```lua
{"item", "player_option", "Mirror"}
```
Name must match a PlayerOptions function.

#### Lua config items
```lua
{"item", player_config, "ScreenFilter", "percent"}
```
Second element must be an object created by create_lua_config.

Next is the name of the option to change.  This can be a path like
"foo.bar.thing", where "thing" inside a table named "bar", which is inside a
table named "foo", which is in the config data.

Fourth element is the glorb type of the option.  This controls how the player
changes the value.

* bool
Option toggles between true and false.
* number
Number adjusted up or down by 1.
* small_number
Adjusted by .1.
* large_number
Adjusted by 10.
* percent
Value is displayed as x%, adjusted by 0.01.
* millisecond
Value displays as xms, adjusted by 0.001.
* time
Value displays as m:ss (minutes, seconds), adjusted by 10.
* toggle_number
```lua
{"item", notefield_prefs_config, "reverse", "toggle_number", {on= -1, off= 1}}
```
The option is displayed as a bool, and the player can toggle between the
values.
* choice
```lua
{"item", misc_config, "transition_type", "choice", {choices= transition_type_choices}}
```
choices must be a table of values for the player to choose from.
* name_value_pairs
```lua
{"item", misc_config, "nvram", "name_value_pairs", {choices= {
{"high", 4}, {"medium", 2}, {"low", 1}}}}
```
Similar to "choice", but each choice is a name,value pair.  The name is
displayed, and the option is set to the value.

#### Data config item
```lua
{"item" some_table, "foo", "number"}
```
Similar to a lua config item.  some_table can be any table.

### custom
```lua
{"custom", {
	name= "April Fools", func_changes_value= true, func= function()
	april_fools= not april_fools return {"boolean", april_fools and true or false} end,
	value= function() return {"boolean", april_fools and true or false} end}},
}
```

A completely custom menu item.  Refer to Menu Item Technical Details.


## Menu Item Technical Details

Each menu item has functions that control what the parts do.
```lua
{
	name= "foo",
	arg= {},
	value= function(arg, pn) end,
	dont_translate_name= false,
	dont_translate_value= false,
	translation_section= "OptionNames",
	type_hint= {main= "item", sub= "custom"},
	func= function(big, arg, pn) end,
	func_changes_value= false,
	adjust= function(dir, big, arg, pn) end,
	reset= function(arg, pn) end,
}
```

* name
The name is passed to the menu item actor to display.  If name is a string
and dont_translate_name is false or nil, the name will be translated before
being passed to the actor.

* arg
Whatever you want to be passed to the value, func, adjust, and reset
functions.
arg will not be modified by the menu system in any way.

* value
A function that returns the value for the actor to display.
If the returned value is a table, and value[1] is "string" and
dont_translate_value is false or nil, value[2] will be translated and the
untranslated value will be put in value.untranslated.
```lua
-- "foo" will not be translated, because it is not a table.
value= function(arg, pn) return "foo" end
-- 5 will not be translated, because "number" is not "string".
value= function(arg, pn) return {"number", 5} end
-- "bar" will be translated.
value= function(arg, pn) return {"string", "bar"} end
```

* translation_section
Overrides the translation section set in the menu controller.

* type_hint
Passed to the actor's SetTypeHintCommand.  In practice, mostly used to mark
submenus.  The generic_menu in _fallback relies on type_hint always being a
table with the main field.

* func
Function to call when the player presses Start or clicks the item.  
big is true if the player holds the button long enough.  
arg is arg, from the item.  
pn is the player number the menu is for.  May be nil.  
This is used to make submenus, explained below.

* func_changes_value
If func_changes_value is true, then func does not open or close or refresh
submenus.

* adjust
Function to call when the player changes the value by clicking an adjuster or
pressing left or right.

dir is 1 or -1, for whether the value should increase or decrease.  
big is true if the player holds the button long enough.  
arg is arg, from the item.  
pn is the player number the menu is for.  May be nil.

* reset
Currently reset can only be activated by middle clicking the value.
The value is set to whatever the reset function returns.

### Submenus with item.func
If func returns "'submenu', {}', the table is a submenu of items to open.

If func returns "'close'", the current submenu closes.

If func returns "'close', 2", the current and previous submenus close.

If func returns "'close', -1", all submenus close, leaving the player at the
top level menu.

If func returns "'refresh', {}", the items in the table replace the current
items.

If func returns "'close', 2, {}", two menu levels close, then the items in
the table replace the ones at the new current level.

The submenu table can have on_open and on_close functions, called when the
submenu opens and closes.
```lua
{
	on_open_arg= {},
	on_open= function(on_open_arg, pn) end,
	on_close_arg= {},
	on_close= function(on_close_arg, remembered_menu_pos, pn) end,
}
```

When creating a submenu this way, you must manually add the close item.
Use nesty_menus.add_close_item, which can be configured to place the close
item at the top or bottom, and with a default close text.
nesty_menus.set_close_default_to_top(true) sets whether add_close_item adds
at the top or bottom.
nesty_menus.set_default_close_text sets the name of the item.

Also when using a custom item to make a submenu, the ```{"item", "preference", "BGBrightness"}``` item form will not work, unless you use
nesty_menus.make_menu(items) to process the items.

It's simpler to use add_close_item after make_menu, like this:
```lua
return "submenu", nesty_menus.add_close_item(nesty_menus.make_menu(items))
```
