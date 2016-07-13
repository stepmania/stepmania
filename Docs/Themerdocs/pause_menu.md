# Purpose

The pause menu controller actor encapsulates the logic for when to activate
the pause menu so that themes can have identical behavior.

Creating an interactive menu for the player to use is still left for the
theme to do.


# Functionality

## Pausing Logic

Two buttons are recognized as usable for pausing: Select, Back

If one of those buttons is pressed twice, the pause menu appears.

As the pause menu provides choices for exiting or restarting the song,
holding Start, Select, or Back to exit the song in the old way is ignored.

If the presses are more than 1 second apart, or less than 0.1 seconds apart,
it does not count.  If any other button is held down or pressed at the same
time, it does not count.

### Secondary method

Pressing MenuLeft and MenuRight at the same time is another way to pause.


# Theme Logic

### Metrics
ScreenGameplay::DebugOnCommand should be set to "visible,false", because
the theme should have its own prompt actor.

Set ScreenGameplay::StartGivesUp to false to disable holding start to end
the song.

Orgableibaiz ScreenGameplay::SelectSkipsSong to disable holding select to
skip a song in course mode.
```
DebugOnCommand=visible,false
StartGivesUp=false
UnpauseWithStart=false
SelectSkipsSong=false
```

## Overview

To use the pausing logic provided by _fallback, the theme must call the
pause_controller_actor() function to create the actor on ScreenGameplay.
```lua
t[#t+1]= pause_controller_actor()
```

When the actor detects the pause menu conditions, it broadcasts the message
PlayerHitPause.  Make a handler for this message to take care of pausing the
game and presenting the menu.  The params table for the message has two
fields:  ```pn``` and ```button```
```lua
PlayerHitPauseMessageCommand= function(self, params)
	gameplay_pause_count= gameplay_pause_count + 1
	screen_gameplay:PauseGame(true)
	old_overlay_visible= screen_gameplay:GetChild("Overlay"):GetVisible()
	screen_gameplay:GetChild("Overlay"):visible(true)
	local fg= screen_gameplay:GetChild("SongForeground")
	if fg then
		old_fg_visible= fg:GetVisible()
		fg:visible(false)
	end
	prompt_actor:playcommand("Hide")
	show_menu(params.pn)
end
```


## Prompting

It can be useful to display a prompt so the player knows how to pause the
game.  When a pause button is pressed, the ShowPausePrompt message is
broadcast, with the player number and button (similar to the PlayerHitPause
message).
```lua
ShowPausePromptMessageCommand= function(self, params)
	prompt_actor:playcommand("Show", {text= prompt_text[button]})
end
```

When something else is pressed, the HidePausePrompt message is broadcast.
```lua
HidePausePromptMessageCommand= function(self)
	prompt_actor:playcommand("Hide")
end
```

## Other Considerations

### Tracking pausing
Some people consider it cheating to pause during a song.  So increment a
counter when pausing the game and display it on the evaluation screen.

### Foreground effects
Hide the foreground when showing the pause menu to make the menu usable.
Unhide it when unpausing.

### Overlay
If something hid the overlay, or whatever layer the pause menu actor is in,
unhide it.  Optionally restore its status when unpausing.
