# Purpose

The lua config system is designed to provide a way to save configuration settings on a per-profile and machine wide basis as lua tables.

## Why lua and not ini?
ini is not suited for complex data structures.  ini can only store a single
layer of data, sections with entries.  Lua can store any reasonable data
structure.

## Where is data saved to?
Data that is for a player is saved to their profile, in a different folder
for each theme, in a different file for each config object.  This way, themes
don't have to worry about accidentally stomping on the configuration for a
different theme.  If a player wants to clear their config data for a theme,
they just delete the folder for that theme.

Machine wide data is stored in a folder in the Save folder.

The folder is named this way:
```lua
"/"..THEME:GetCurThemeName().."_config/"
```

So if the theme is named "default", the machine wide config for it is in
"Save/default_config/" and the profile data is in "default_config/" inside
each profile folder.


# Creation

Create a table with the default values for the configuration.  This will only
be used when creating the config object.

Call create_lua_config to create the config object.  create_lua_config
returns a config object, which should be stored in a global variable if you
want it to be reachable everywhere in the theme.

## Parameters to create_lua_config

create_lua_config takes its parameters in the form of a table.  Each
parameter is a named field in the table.

### Required parameters

* file  
The name of the file to save this config object to.

* default  
The table of default configuration.  This is the configuration that will be
loaded when there isn't a saved configuration.  The default table will also
be used to sanity check when loading a configuration.

### Optional parameters

* name  
A name to use for this config.  This will be used by things that need a name
for the config, like the ConfigValueChanged broadcast message.
If the name field is not set, it will be set to the file field.

* use_global_as_default  
If this is true, then when there is no data for a profile, the data in
ProfileSlot_Invalid will be loaded and used instead of the default table.
This can be useful for providing a way for a machine operator to create a
default config that players on the machine use when they haven't set their
own config.

* use_alternate_config_prefix  
This is for bypassing the normal saving to save outside of the theme specific
folder in the profile.

* match_depth  
This is one of the fields that controls how the sanity checking behaves.
Normally, the sanity checking will recurse through the data table, making
sure all fields have the same names and types as the fields in the default
table.  match_depth controls how deep into the data table the sanity checking goes.  Any negative value makes the sanity checking go all the way.  0
disables the sanity checking.  1 makes the sanity checking only check the top
layer.  2 means check the top two layers, and so on.

* exceptions  
This is another field that controls how the sanity checking behaves.  The
exceptions field is a list of string names of fields to not sanity check.
This handles disabling sanity checking for fields that can't be handled by
the match_depth field.

### Sanity checking

Sanity checking is the process of checking loaded data to make sure it fits
within certain limits and is safe to use and makes sense.  The sanity
checking system built into the lua config system is based on simple type
matching.  The data loaded from a config file must have the same fields as
the default config and the field values must have the same types.  If the
loaded data has a field that doesn't exist in the default config, that field
is removed.  If the loaded data lacks a field that is in the default config,
that field is filled with the default value.  If the loaded data has a field
that is in the default config, but the value has a different type, the value
is replaced with the default value.

#### Examples:
Default config for the example:
```lua
local default_foo_config= {
  bar= 1, baz= "quux",
  zeen= {
    torg= 2, dorn= "xug",
  },
}
```

Config file with one field missing:
```lua
local config_data= {baz= "zork", zeen= {torg= 5, dorn= "loo"},}
```
After sanity checking, that config data looks like this:
```lua
config_data= {bar= 1, baz= "zork", zeen= {torg= 5, dorn= "loo"},}
```

Config file with a field that is the wrong type:
```lua
local config_data= {bar= 3, baz= 6, zeen= {torg= 5, dorn= "loo"},}
```
After sanity checking:
```lua
config_data= {bar= 3, baz= "quux", zeen= {torg= 5, dorn= "loo"},}
```

Config file with an extra field:
```lua
local config_data= {bar= 3, yun= "awk", baz= "quux", zeen= {torg= 5, dorn= "loo"},}
```
After sanity checking:
```lua
config_data= {bar= 3, baz= "quux", zeen= {torg= 5, dorn= "loo"},}
```


# Loading and Saving

Machine wide config should be loaded when scripts are loaded, and saved after
changes.  Profile config should be loaded when the profile is loaded and
saved when it's saved.

Doing things when the profile is loaded or saved is normally accomplished by
writing a LoadProfileCustom and SaveProfileCustom functions.  But this
presents a problem when you have several configs in different places and you
want to keep code simple by only referencing the config in the file that
creates it.  Also, a beginning themer might not know a good way to match a
profile with the right config slot to save or load.

The lua config system provides functions to deal with these problems.

## Simple instructions

Do not create LoadProfileCustom or SaveProfileCustom functions anywhere.
Call add_standard_lua_config_save_load_hooks after the config object is
created to register the config to be saved and loaded with the profile.
```lua
add_standard_lua_config_save_load_hooks(some_config)
```
If you have other things to do when saving or loading profiles, read the
Explanation section.

## Explanation

_fallback/Scripts/01 profile_load_save.lua creates LoadProfileCustom and
SaveProfileCustom functions that call a list of callbacks.

add_profile_load_callback adds a function to the list of callbacks to call
when a profile is loaded.

add_profile_save_callback adds a function to the list of callbacks to call
when a profile is saved.

This allows different parts of the theme to add their own loading and saving
functions without disrupting other parts.

The lua config system has standard load and save functions that take care of
matching a profile with a config slot by finding the player that the profile
was loaded for.  These functions then call the load and save functions on the
config object.

add_standard_lua_config_save_load_hooks adds these functions to the callback
lists.

You can add your own loading and saving functions to the callback lists too.
Pass your load function to add_profile_load_callback, and your save function
to add_profile_save_callback.


# Using the config

When you need the config data for a player, call get_data on the config
object and pass it the player number.
```lua
local player_config= foo_config:get_data(pn)
```
If you are getting the machine wide config, pass nil instead of the player
number.
```lua
local machine_config= foo_config:get_data()
```
get_data returns a table with the config values.  If you modify anything in
the table, call set_dirty on the config object with the same arg that you
passed to get_data.
```lua
player_config.bar= 5
foo_config:set_dirty(pn)
```
Calling set_dirty tells the system that the config needs to be saved next
time saving occurs.  The config is only saved when it is dirty to reduce
saving time.


# OptionRows

No lua option rows are provided by the lua config system.  For now, those are
left as an exercise for the reader.


# Nesty Options

The nesty options system provides menus for bool, float, and float toggle
config values.  Read the documentation on that system for an explanation.


# Other useful functions

* split_name(string)  
split_name takes a string and splits it everywhere that a dot ('.') occurs.
This is useful to the get_element_by_path and set_element_by_path functions.

* get_element_by_path(container, path)  
get_element_by_path finds the element in the container that matches the path.
This is useful when creating some function that needs to fetch and set an
element in a table, and users of the function need to be able to specify a
multi-step path instead of being limited to the top level of the table.
Some of the menus in the nesty options menu system use get_element_by_path
to set the fields for the values they control.
If any stage of the path does not exist, get_element_by_path returns nil.

* set_element_by_path(container, path, value)  
set_element_by_path works similarly to get_element_by_path.  It is used to
set a value.
If any stage of the path does not exist, set_element_by_path will create it.

* string_in_table(str, tab)  
Finds a string in an array of strings and returns its index.  Returns false
if the string cannot be found.

* table_find_remove(tab, thing)  
string_in_table combined with table.remove.


# Origin Story

The lua config system was written by Kyzentun in 2014 for his Consensual
theme.  The existing ThemePrefs and UserPrefs systems were deemed unsuitable
because they use ini and were probably not equipped for handling complex
structured data that might change in format over time.  Also, creating 
replacements is more fun than carefully examining old things to make sure
they don't have weird problems.

Since then, the system has changed little, with small additions as needed.
