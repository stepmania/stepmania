ThemePrefs, GamePrefs, UserPrefs, and CustomSpeedMods are all obsolete and
have been removed.

The lua_config system should be used to replace anything that used
ThemePrefs, GamePrefs, or UserPrefs.

Flexible speed mod settings are provided by using the newfield prefs and the
nesty options menu system.


## Removed Function Names
If your theme uses any of the functions listed below, that part of the theme
needs to be updated.

### ThemePrefs
* GetThemePref
* SetThemePref
* ThemePrefRow

### CustomSpeedMods
* load_custom_speed_mods
* SpeedMods
* SpeedModIncSize
* SpeedModIncLarge
* GetSpeedModeAndValueFromPoptions
* ArbitrarySpeedMods

### GamePrefs
* ReadGamePrefFromFile
* WriteGamePrefToFile
* GetGamePref
* SetGamePref
* GetGamePrefB
* GetGamePrefC
* GetGamePrefN

### UserPrefs
* ReadPrefFromFile
* WritePrefToFile
* GetUserPref
* SetUserPref
* GetUserPrefB
* GetUserPrefC
* GetUserPrefN

### Other
* InitGamePrefs
* InitUserPrefs
* GetProTiming
* OptionRowProTiming
* GetDefaultOptionLines
* OptionRowScreenFilter
* OptionRowProTiming
* GetTapPosition
* ComboUnderField


## Removed Table Names
If your theme uses any of the functions listed below, that part of the theme
needs to be updated.

### ThemePrefs
* ThemePrefs.NeedsSaved
* ThemePrefs.Init
* ThemePrefs.Load
* ThemePrefs.Save,
* ThemePrefs.ForceSave
* ThemePrefs.Get
* ThemePrefs.Set
* ThemePrefs.InitAll

### ThemePrefsRows
* ThemePrefsRows.IsInitted
* ThemePrefsRows.Init
* ThemePrefsRows.GetRow
