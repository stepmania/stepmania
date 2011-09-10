Hello World
and other sm-ssc theming examples
-------------------------------------------------------
 A theme by various contributors. (see "Contributors")
-------------------------------------------------------
This theme and its accompanying documentation are licensed under
the Creative Commons Attribution-Share Alike 3.0 license.

The license text can be viewed at:
http://creativecommons.org/licenses/by-sa/3.0/us/

================================================================================
Table of Contents
 Introduction [intro]
 Chapter List [chapt]
  Guide       [guide]
   Beginning   [g_beg]
   Metrics     [g_met]
   Fonts       [g_fon]
   Graphics    [g_gfx]
   BGAnims     [g_bga]
   Sounds      [g_sfx]
   Scripts     [g_scr]
   Other       [g_oth]
   Debug Menu  [g_deb]
   NoteSkins   [g_not]
   Announcers  [g_ann]
   Characters  [g_cha]
  Reference   [ref]
   Screens     [r_scr]
   Managers    [r_man]
   Classes     [r_cls]
  Tutorials    [tutor]
 Contributors [creds]
[others tba]

================================================================================
Introduction [intro]

Welcome to Hello World (and other sm-ssc theming examples).
This theme and accompanying documentation are provided for people new to
theming for StepMania, using sm-ssc as a base.

================================================================================
The Chapters [chapt]

The theme itself is split into three sections, encompassing various elements of
sm-ssc theming.

The Guide is a general run-through of how Themes are constructed (along with
NoteSkins, Announcers, and Characters)
================================================================================
Guide [guide]
This section covers the basics of theming for StepMania.

--------------------------------------------------------------------------------
Beginning [g_beg]

So you've decided to make a theme for StepMania 5.
This section will tell you how to get started.

The most important rule: NEVER COPY THE DEFAULT THEME'S FOLDER.
This will only hurt you in the end, as the default theme is updated a lot, and
any changes made in it WON'T carry over to your theme.

The recommended way is to make a blank folder and use the fallback system,
falling back on _fallback. If you are making an edit of the default theme,
then you can set the fallback value to default. This will be explained more
in the Metrics section.

So hopefully by now you've decided to start from a blank folder.

Make two blank text files: metrics.ini and ThemeInfo.ini.
Then make these folders:
BGAnimations/, Fonts/, Graphics/, Languages/, Other/, Scripts/, Sounds/

# ThemeInfo.ini
The ThemeInfo.ini file from the default theme looks like this:
[ThemeInfo]
DisplayName=Default
Author=Midiman

It should be straightforward enough to follow and edit.
Once you've done this, you can move on to the next section.

--------------------------------------------------------------------------------
Metrics and Languages [g_met]

Both the metrics and languages use the .ini format, so they're being lumped
together. If you're not familiar with an .ini file, here's the basics:

# comment
[Section]
Key=Value

This is simple enough for the languages, but the metrics require some more
knowledge to work with.

--------------------------------------------------------------------------------
Fonts [g_fon]

--------------------------------------------------------------------------------
Graphics [g_gfx]

--------------------------------------------------------------------------------
BGAnimations [g_bga]

BGAnimations are the heart of any theme. Ever since late 2007, they've been
created using Lua.

--------------------------------------------------------------------------------
Sounds [g_sfx]

--------------------------------------------------------------------------------
Scripts [g_scr]

Though Lua scripting has been available in StepMania since 2006, the current
iteration is a lot more powerful than before. This is mainly due to the
additional bindings added during sm-ssc (carried over to SM5).

--------------------------------------------------------------------------------
Other [g_oth]

Ever wonder what all those files in the Other folder are for?

--------------------------------------------------------------------------------
Debug Menu and Shortcuts [g_deb]

The debug menu will be your best friend when theming.

Access the debug menu by holding F3. There are three sections of the menu
available: Main (F5), Theme (F6), and Profiles (F7).

There are other shortcuts that will be helpful for theming:
Tab - speeds up animations.
`/~ - slows down animations.
Tab + `/~ - freezes animations.
F2 - Reloads theme and textures.
Shift + F2 - Reloads metrics only.
Ctrl + F2 - Reloads scripts only.
Ctrl + Shift + F2 - Reloads the overlay screens.
F1 - Insert Coin
Print Screen - Take a JPEG screenshot.
Shift + Print Screen - Take a PNG screenshot.

--------------------------------------------------------------------------------
NoteSkins [g_not]

jousway is probably gonna have to work on this.

--------------------------------------------------------------------------------
Announcers [g_ann]

Announcers are somewhat of a lost art.

--------------------------------------------------------------------------------
Characters [g_cha]

This guide won't teach you how to model or animate, sorry.

================================================================================
Reference [ref]
This section contains a reference of various elements in StepMania.
For a reference of Lua bindings, check http://kki.ajworld.net/lua/sm5/Lua.xml

================================================================================
Contributors [creds]

These are the people who have worked on Hello World:
* AJ Kelly