# Enabling

If the theme you're using has ```BGAnimations/ScreenEdit overlay.lua```, add
the preview actor to it like this:
```lua
t[#t+1]= LoadActor(THEME:GetPathG("", "editor_mods_preview.lua"))
```

If ```BGAnimations/ScreenEdit overlay.lua``` does not exist, create it with
only these lines in it:
```lua
return Def.ActorFrame{
	LoadActor(THEME:GetPathG("", "editor_mods_preview.lua")),
}
```

The fallback and default themes do not load the mods preview to avoid
surprising people who don't expect or want it.


# Controls

Even when loaded, the preview is hidden by default.  Press keypad 1 to unhide
it.

There are clickable buttons for controlling the preview that are hidden when
the mouse is not near them because the edit mode screen has too much stuff on
it already.

The buttons are in the upper left, where the preview defaults to.  If the
preview is moved to a different corner, the buttons move with it.

When not paused, the preview plays the same part of the song in a loop.
The time that the main edit field is at is the current time.  The preview
plays from current_time + min_offset to current_time + max_offset.  So when
min_offset is -1, and max_offset is 1, the preview will play from one second
before current time to one second after, then loop.

Left click on a button to change it by a small amount.  Right click changes
by 10x the small amount.  Scroll the mouse wheel while the cursor is over the
button (not the number) to change the value.  Middle click will reset it to
the default.

Keypad 0 is bound to reload the mods file.

Keypad 1 is hides and reveals the preview.

Keypad 2 toggles pausing.

Keypad 3 toggles full screen.

Keybindings can be changed by editing Save/editor_config.lua until a binding
gui is added.


# Mods file

The preview loads up the file in the current song folder named
```notefield_mods.lua```.  So if the current song is "MechaTribe Assault",
it will load "MechaTribe Assault/notefield_mods.lua".

The format of the mods file is described by simfile_mods.md.
