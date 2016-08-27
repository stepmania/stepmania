# Noteskin stuff
* Change "NewSkinTapPart_whatever" to "NoteSkinTapPart_whatever".
* Change NEWSKIN to NOTESKIN.
* Change newfield to notefield.
* Change your waifu.

# Easy stuff

## Removed metrics
* ArrowEffects::  
  Everything in the ArrowEffects section has been removed.  All they did was
	screw up mods.
* Common::DefaultNoteSkinName  
  Why should the theme control the default noteskin?  That should be the
	player's choice.
* NoteField::FadeBeforeTargetsPercent  
  Useless.
* NoteField::FadeFailTime  
  Useless.
* NoteField::ShowBoard  
  Board doesn't exist in NewField, layers in NoteField layers should be
	controlled by prefs.
* NoteField::ShowBeatBars  
  Useless.  Only edit mode needs beat bars.
* NoteField::BarMeasureAlpha  
  Useless.
* NoteField::Bar4thAlpha  
  Useless.
* NoteField::Bar8thAlpha  
  Useless.
* NoteField::Bar16thAlpha  
  Useless.
* NoteField::RoutineNoteSkinP1
* NoteField::RoutineNoteSkinP2  
  Handling of noteskins in routine mode is different, and this never should
	have been a theme metric.
* Player::ReceptorArrowsYStandard
* Player::ReceptorArrowsYReverse
* Player::DrawDistanceAfterTargetsPixels
* Player::DrawDistanceBeforeTargetsPixels  
  Controlled by the NoteField now. (and per-column)
* Player::ComboUnderField
* Player::TapJudgmentsUnderField  
  Controlled by draw order.
* Player::HoldJudgmentsUnderField  
  Hold judgments belong in NoteColumn layers.
* ScreenEdit::InvertScrollSpeedButtons  
  Useless.
* ScreenGameplay::MarginFunction  
* ScreenGameplay::PlayerP1OnePlayerOneSideX
* ScreenGameplay::PlayerP2OnePlayerOneSideX
* ScreenGameplay::PlayerP1TwoPlayersTwoSidesX
* ScreenGameplay::PlayerP2TwoPlayersTwoSidesX
* ScreenGameplay::PlayerP1OnePlayerTwoSidesX
* ScreenGameplay::PlayerP2OnePlayerTwoSidesX
* ScreenGameplay::PlayerP1TwoPlayersSharedSidesX
* ScreenGameplay::PlayerP2TwoPlayersSharedSidesX
  Replaced by Player1MiscX Player2MiscX and PlayerPositionFunction, which
	should scale better and be style agnostic.
* ScreenNameEntry::
  Setting high score name with letters scrolling in a notefield is no longer
	supported in the engine.  You can probably do it in lua if you're clever
	and desparate enough.
* ScreenOptionsMaster::NoteSkinSortOrder
  Useless.

## Added metrics
* ScreenGameplay::Player1MiscX
* ScreenGameplay::Player2MiscX
* ScreenGameplay::PlayerPositionFunction  
  Style agnostic way to position players, zoom the notefield to fit, and cook
	your dinner.

## Removed lua functions
* ArrowEffects:  
  Bad for your health.  Reimplementing notefield functionality in lua is not
	a good solution to any problem.
* NOTESKIN:  
  All old NOTESKIN functions have been removed.
* GAMESTATE:RefreshNoteSkinData  
  Should not have been part of GameState.
* Style:GetWidth  
  Width is controlled by the noteskin and must be fetched from the notefield.
* use_newfield_on_gameplay  
  Useless.
* Removed NoteSkin and NewSkin fields from PlayerOptions structure.
  NoteSkin is set in the profile now.

* Renamed lua functions
* use_newfield_actor -> notefield_prefs_actor  
  All it does is apply the notefield prefs provided by the fallback theme.

## Removed fonts
These were used by the old ScreenNameEntry that got removed.
* ScreenNameEntry letters
* ScreenNameEntry category
* ScreenNameEntry step

## Removed preferences
* DefaultNoteSkin

## Removed dirs
* NewSkins  
  NoteSkins belong in the NoteSkins folder.

## Misc
* Removed column positions from Style.


# Fun stfu

## ScreenEditMenu

It's a nesty menu just like ScreenNestyPlayerOptions.  Default theme shows
how to make some simple actors for showing info about the current song.

## Graphics/ScreenEdit option_menu

Another nesty menu.  All you do is set the params for displaying it, and some
actor to go behind it to look nice.
