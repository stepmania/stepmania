 ____    __                                                             
/\  _`\ /\ \__                 /'\_/`\                   __             
\ \,\L\_\ \ ,_\    __   _____ /\      \     __      ___ /\_\     __     
 \/_\__ \\ \ \/  /'__`\/\ '__`\ \ \__\ \  /'__`\  /' _ `\/\ \  /'__`\   
   /\ \L\ \ \ \_/\  __/\ \ \L\ \ \ \_/\ \/\ \L\.\_/\ \/\ \ \ \/\ \L\.\_ 
   \ `\____\ \__\ \____\\ \ ,__/\ \_\\ \_\ \__/.\_\ \_\ \_\ \_\ \__/.\_\
    \/_____/\/__/\/____/ \ \ \/  \/_/ \/_/\/__/\/_/\/_/\/_/\/_/\/__/\/_/
                          \ \_\                                         
                           \/_/                                         
                           __         __     
                         /'__`\     /'__`\   
                        /\_\L\ \   /\ \/\ \  
                        \/_/_\_<_  \ \ \ \ \ 
                          /\ \L\ \__\ \ \_\ \
                          \ \____/\_\\ \____/
                           \/___/\/_/ \/___/ 

(c)2001-2002, StepMania team
http://www.stepmania.com

************************************************************
Table of Contents
************************************************************

1. About StepMania
2. Minimum Requirements
3. DirectX 
4. Installation 
5. Controls
6. How StepMania Loads Songs
7. Connecting Dance Pads 
8. Compatibility Issues
9. FAQ / Troubleshooting
10. StepMania Packages - .smzip
11. Questions, Bugs, Suggestions, and Help 
12. The StepMania Editor
13. The .SM File Format
14. Building StepMania from CVS Source
15. Version History

************************************************************
1. About StepMania
************************************************************

StepMania game system is capable of playing many game types.  Currently, it supports games Dance Dance Revolution and Pump It Up, with future support for EZ 2 Dancer, BeatMania, Guitar Freaks, DrumMania, Para Para Paradise and more.

************************************************************
2. Minimum Requirements
************************************************************

* Windows 98, Windows ME, Windows 2000, or Windows XP.
* Required: Pentium II, Pentium III, Celeron, Athlon, or compatible processor, 266MHz minimum (400 MHz highly recommended).
* 64 MB of RAM.
* Video card that supports High Color (16-bit color) and has 16MB video RAM.
* DirectX 8.1 compatible video card with drivers.
* DirectX 8.1 compatible sound card with drivers.
* DivX 5 video codec (for movie visualizations).

************************************************************
3. DirectX
************************************************************

DirectX 8.1 is required before installing StepMania. If DirectX 8.1 is not installed on your computer, the StepMania installer will display a warning and abort the installation.

************************************************************
4. Installation
************************************************************

To install StepMania, download the file stepmania3.exe to a known location on your hard drive, then double-click the file icon to begin the installation. The installer automatically remove an older version of the program if one is present. The default installation directory is 'C:\Program Files\StepMania'.

************************************************************
5. Controls
************************************************************

Use the "Config Key/Joy" option from the Title Menu to configure your keyboad or dance pad.

Any time:
  * F4 = toggle fullscreen
  * F5 = toggle detail

In menus:
  * Arrow keys navigate menus
  * Enter = Start, Escape = Back

In gameplay:
  * F11 = decrease offset
  * F12 = increase offset

************************************************************
6. How StepMania Loads Songs
************************************************************

All song files for a specific song (data file, music file, banner, background) must reside in a single folder.  This folder must have the following path:

   Songs/<group folder>/<song folder>/

For example, the song B4U may consist of the following files:

   Songs/Dance Dance Revolution 4th Mix/B4U/b4u.dwi
   Songs/Dance Dance Revolution 4th Mix/B4U/b4u.mp3
   Songs/Dance Dance Revolution 4th Mix/B4U/b4u-banner.png
   Songs/Dance Dance Revolution 4th Mix/B4U/b4u.png

************************************************************
7. Connecting Dance Pads
************************************************************

There are several devices that allow you to connect PlayStation dance pads to your PC.

* Not recommended:  generic PlayStation->USB converter 

Using a generic PlayStation->USB converter is not recommended to play any dancing game on the PC.  The vast majority of these converters are not capable of registering simultaneous presses of Left+Right or Up+Down.  This restriction will cause the player to miss many notes where two buttons must be hit simultaneously.

* Highly Recommended:  Special PlayStation->USB adaptor

There is just one brand of PlayStation->USB adaptor that functions perfectly with StepMania.  These adaptors are available for purchase at the StepMania web site (www.stepmania.com) and through some importers (www.liksang.com).  The adaptor is made by “Boom”, and it small and black with a detachable USB cable.  These adaptors cost about $15 and allow you to attach one dance pad per adaptor.  The primary advantage of these adaptors is that they do not require special drivers and require no configuring.

* Semi-Recommended:  PlayStation->Parallel adaptor

PlayStation->Parallel converters do not have the simultaneous button problem that USB converters do.  The disadvantage of these adaptors is that home-made drivers are required to use these devices.  These drivers are often finicky and incompatible with some sound cards.  PlayStation->Parallel adaptors can be purchased from the StepMania web site or any importer.

For Windows 98 and Windows Me users, the recommended driver is “DirectPad Pro 5.0 for Win9x”.  For Windows 2000 and Windows XP, the recommended drivers are NTPad XP 1.x.  Both of these drivers can be downloaded from Aldos web site: http://www.aldostools.com/dpad.html

************************************************************
8. Compatibility Issues
************************************************************

Some video card/driver combinations have visual errors when running StepMania. If you experience any visual errors, please visit your video card manufacturer's web site and download the latest Direct3D 8.1 drivers. Below is a list of known issues that are thought to be video card driver bugs:

* S3 Savage family cards (e.g. Diamond Stealth)
Symptom: The right edge of some graphics appears "cut off".
Explanation: Drivers do not properly handle textures > 512x512, resulting in cropped textures. Please use the Low texture detail option to force texture to be 512x512 or smaller.

* SiS cards 
Symptom: Garbled fonts.
Explanation: Drivers have errors in handling texture coordinates. Only known solution is to upgrade drivers.

* Any card older than two years
Symptom: Various graphical glitches.
Explanation: These problems are most often video card driver bugs.  Please upgrade to the latest drivers, which will hopefully fix the errors.


************************************************************
9. FAQ / Troubleshooting
************************************************************

* The error "Cannot find Dinput8.dll" appears when launching the game.

You need to download DirectX 8.1.  http://www.microsoft.com/directx

* Music rates of over 100% do not work on my computer. Why?

Some very old sound cards do not support sound playback rates that are greater than the original sample rate.  This problem is likely to be corrected by newer sound card drivers.

* StepMania reports “Direct3D Create Device failed”.

This error means that your video card or drivers do not support Direct3D 8.  Download the latest drivers for your video card.  If your video card is older than 2 or 3 years, Direct3D 8 drivers may not be available.  Sorry.

* I’m looking to purchase a video card.  What works well with StepMania?

Just about any video card manufactured in the last year will run StepMania flawlessly at 60 frames per second.  I recommend any card with an nVidia GeForce chip or an ATI Radeon chip.

* How can I improve performance with my existing video card

1. Install the latest drivers for your video card.
2. Press F5 in the game to toggle between high detail and low detail modes.

************************************************************
10. StepMania Packages - .smzip
************************************************************

The StepMania package format was created to make the distribution of songs and other add-ons very easy.  StepMania package files have the extension '.smzip' and can be installed by double-clicking the .smzip file.

A StepMania package is 'installed' by extracting all files in the package to the StepMania program directory. This allows songs, courses, themes, and visualizations to all be installed by the Package Manager.

The file format of an .smzip file is actually the PK-Zip standard.  This means you can rename any .smzip file to have the extension '.zip', and then open the file in any compression package (e.g. WinZip).

The StepMania Package Manager (smpackage.exe) can also create song packages for you.  Simply launch the Package Manager (Start Menu->Programs->StepMania->Package Manager), click the song you want to create a package for, click the Export button, and a .smzip package will be placed on your desktop.

************************************************************
11. Questions, Bugs, Suggestions, and Help
************************************************************

If you have a question or problem about StepMania, please try asking your question to the StepMania message board.  A link is available from the StepMania web site – http://www.stepmania.com.

If you have found a bug, please file a bug report into the SourceForge bug tracking system (http://sourceforge.net/tracker/?group_id=37892&atid=421366).  This will allow the developers to track and fix bugs much more efficiently.  

* In a bug report, please specify your video card brand and model, and your OS version.
* PLEASE, PLEASE enter your e-mail address on the bug reports. In many cases, we need more information from the bug reporter.

Please use SourceForge's feature request system to enter new feature requests (http://sourceforge.net/tracker/?atid=421369&group_id=37892&func=browse).  Also, please discuss feature ideas on the StepMania message board or the StepMania developers list (stepmania-devs@sourceforge.net).  

If you would like to help with development of StepMania, please send an email to the StepMania developers list (stepmania-devs@sourceforge.net) or to Chris (chris@videogamers.com).

************************************************************
12. The StepMania editor
************************************************************

The StepMania editor allows you to edit, record, synchronize, and play back note patterns.  Current, it support saving only in the .SM format.  If you save a song that was originally in DWI or BMS format, the old DWI and BMS files will be renamed to have ".old" appended to avoid conflicts when the song is next loaded.  All of the commands are listed along the left side of the screen.

************************************************************
13. The .SM file format
************************************************************

The .SM song file format was created to be one file format that supports all game types that StepMania can play (dance, pump, beat, guitar, etc).  The .SM file syntax is identical to DWI except for some different tags and meanings.  These are a list of all supported tags.

All images can be in png, gif, jpg, or bmp format.  All sounds can be in ogg, mp3, or wav format.


#TITLE:...;    - The "main title" of the song.
#SUBTITLE:...; - This text will appear underneath the main title of the song on the Select Music screen.  e.g. "-Dirty Mix-", "(remix)".
#ARTIST:...;   - The artist of the song.
#CREDIT:...;   - Give yourself some credit here for creating a wonderful song.
#BPM:...;      - BPM of the music
#BANNER:...;   - The file name of the banner image.  e.g. "b4u-banner.png".  This image must reside in the song folder.
#BACKGROUND:...;   - The file name of the background image.  e.g. "b4u-bg.png".  This image must reside in the song folder.
#CDTITLE:...;   - The file name of the spinning CD logo.  e.g. "b4u-cdtitle.png".  This image must reside in the song folder.
#MUSIC:...;   - The file name of the music file.  e.g. "b4u.mp3".  This image must reside in the song folder.
#MUSICBYTES:...;   - The size in bytes of the music file that this song was last synchronized with.  A "Song may be out of synch" warning will be displayed if the value of this tag does not match the actual size of the music file.
#OFFSET:...;   - The time in seconds at which beat 0 occurs in the music.  This is specified as a floating point value.  e.g. "2.34".
#SAMPLESTART:...;   - The time in seconds to start the music sample that plays on the Select Music screen.  This is specified as a floating point value.  e.g. "32.34".
#SAMPLELENGTH:...;   - The time in seconds let the sample music play after starting.  This is specified as a floating point value.  e.g. "32.34".  Note that in the last 1 second of playing the music will fade out.
#FREEZE:...;   - A value of the format "beat=sec".  Indicates that at 'beat', the motion of the arrows should stop for "sec" seconds.  Both of these values are specified as floating point values.  Multiple freezes can be given by separating them with commas.  e.g. "60=2.23,80=1.12".

#BPMS:...; - A value of the format "beat=bpm".  Indicates that at 'beat', the speed of the arrows will change to "bpm".  Both of these values are specified as floating point values.  You must specifiy a BPM value for beat 0.  Multiple freezes can be given by separating them with commas.  e.g. "0=160,120=80".  


Each pattern of steps for different modes have the same basic format:

#NOTES:(NotesType):(Description):(DifficultyClass):(DifficultyMeter):(RadarValues):(NoteData);

NotesType:  Must be one of the currently supported types in StepMania: "dance-single", "dance-double", "dance-couple", "dance-solo", "pump-single", "pump-double".

Description:  This will be displayed on the gameplay screen.  This can be any text, but is most commonly "Basic", "Another", "Trick", "Standard", "SSR", "Maniac", "Heavy", or "Challenge".

DifficultyClass:  This value must be "easy", "medium", or "hard".  These values correspond the levels of difficulty on the Select Difficulty screen.

DifficultyMeter:  The difficulty of these notes as a bar rating.  The value must be an integer between 1 and 10.

RadarValues:  The values that will be displayed on the Groove Radar as floats between 0.00 and 1.00.  The 5 values must be separated by commas, and are specified in the order: "stream,voltage,air,freeze,chaos".  For MAX300, these values are "0.95,0.98,0.22,0.12,0.13".  These values will automatically be written when you save the song in the StepMania editor, but you may edit the .SM file afterward to tweak them.

NoteData:  

,  // measure 32
1000
0000
0000
0001
0000
0000
1000
0000
,  // measure 33
0010
0100
0010
0000


************************************************************
14. Building StepMania from CVS source
************************************************************

CVS is the repository that manages the StepMania source code.  Our CVS server is graciously provided by SourceForge.  You can get the absolute-latest bleeding edge updates to StepMania by taking the source from CVS.  Below are instructions on how to configure your computer to download source code from CVS and complile the source into a working executable.

* Step 1: Install and configure Visual C++

The StepMania source can currently be built with Microsoft Visual C++ 6.0 or Visual C++ .Net.  Installing Visual C++ or Visual Studio with all the default options is sufficient.

* Step 2: Install the Direct X 8.1 SDK

The DirectX 8.1 SDK is a free (but large) download from Microsoft (link here).  Install it using all the default settings.  The SDK installer will configure Visual C++ with the correct paths needed to find the DirectX 8.1 libraries and headers.

* Step 3: Install WinCVS

A CVS client is required to "checkout" the course from the CVS server.  Download version 1.2 of WinCVS here - http://www.cvsgui.org.
IMPORTANT: Now, open and close WinCVS once.  This will cause WinCVS to create registry entries, which is required for the next step...

* Step 4: Use SourceForge Setup to install SSH and configure WinCVS

A small utility called SourceForge Setup will greatly simplify the steps needed to configure SSH and WinCVS.  Download SourceForge Setup here - link here.  Use the following settings when prompted by SF Setup:
- Unpack SSH directory: c:\ssh
- SSH install direcotory: c:\ssh
- Your home directory: c:\ssh
- Your SourceForge user name: -yourusername-
- Name of the project: stepmania
- The directory where you will be keeping checked out project sources: c:\stepmania
IMPORTANT: If you are using Windows 2000 or Windows XP SourceForge Setup will not correctly write changes to the c:\autoexec.bat file.  Verify that the following lines are present after you run SF Setup:

rem SourceForge setup stuff
SET PATH=%PATH%;c:\ssh
SET HOME=c:\ssh

IMPORTANT: You must restart after running SF Setup so that the new environment variables take effect.

* DONE! Enjoy staying on the bleeding edge of StepMania releases! 

************************************************************
15. Version History
************************************************************

------------------------Version 3.00 beta 1------------------
NEW FEATURE:  Roulette
NEW FEATURE:  AutoPlay (selectable from Game Options screen)
NEW FEATURE:  Read Delight BMS 0.5 (hold notes, freezes, and new BPM changes supported)
NEW FEATURE:  Editor - edit, record, playback, and synchronize from one screen
NEW FEATURE:  Oni Nonstop Challenge (not finished)
NEW FEATURE:  "Judge Window" option to adjust difficulty of step judging
NEW FEATURE:  Song data cache for faster loading.
NEW FEATURE:  Groove Radar
NEW FEATURE:  Dark Mode (selectable from Player Options screen)
NEW FEATURE (partially complete):  Pump It Up support
NEW FEATURE (partially complete):  Background Animations (defaut animations go in BGAnimations dir)

------------------------Version 2.90-------------------------
CHANGE:  Ogg Vorbis music support re-enabled
NEW FEATURE:  Note Skins (selectable from Player Options screen)

------------------------Version 2.50-------------------------
NEW FEATURE:  New multi game compatible .sm file format.
CHANGE:  "Graphic Detail" presets
BUG FIX:  Installer DirectX check now works
NEW FEATURE:  Aracde and Free Play game modes

------------------------Version 1.90-------------------------
OPTIMIZATION:  Major speedup to BitmapText loading
CHANGE:  Menus changes to accomodate MAX-style menus
CHANGE:  Now compiles in Visual C++.Net
OPTIMIZATION:  Max texture size and texture color options are now supported.
BUG FIX: Scaled sprite alignment (no more transparent strip along the edge of "danger" static)
NEW FEATURE:  Repeating input
NEW FEATURE:  Select Group menu
NEW FEATURE:  Edit / Record / Playback
NEW FEATURE:  Support for DWI solo notes in DWI reader

------------------------Version 1.65-------------------------
BUG FIX:  Statistics now save and load correctly
BUG FIX:  Better framerate during gameplay

------------------------Version 1.64-------------------------
NEW FEATURE:  Group banners on Select Music screen.  Just put a banner in the group folder.
BUG FIX:  Hacked around a Savage texture loading bug.
CHANGE:  Added "hold NEXT for options" after choosing a song.  Most people don't know about this feature!
CHANGE:  Faster song loading
CHANGE:  more accurate synching at song rates other than 100%
CHANGE:  Assist tick calibrated automatically by detecting sound card play latency
CHANGE:  Lowered wait time between menus
CHANGE:  Improved loading time of sounds
BUG FIX:  Improved quality of sounds while dancing
BUG FIX:  Fixed DWI parsing error that often caused the last note to not appear
NEW FEATURE:  More resolution presets in GameOptions menu

------------------------Version 1.63-------------------------
NEW FEATURE:  Effects "drunk", "dizzy", "space"
CHANGE:  Song sort by group now has collapsible sections
BUG FIX:  Prefs, songs, and default mappings now load if StepMania.ini doesn't exist
BUG FIX:  Pad 2 can now be used to navigate the title screen
BUG FIX:  Crash if change sort while a section item was selected
BUG FIX:  BPM changes to 0 when section is selected
BUG FIX:  Pixel/Texel alignment bug causing a strip of color along right and bottom edges moving between menus.
BUG FIX:  Graphical glitches on Results screen
BUG FIX:  Hold Steps now look better when Scroll Speed is > 1
BUG FIX:  Wrong Ghost Arrow graphic being used
BUG FIX:  Boost/Scroll Speed interaction is now like MAX
BUG FIX:  DWI's with newlines before semicolon are no longer corrupt when saving using Synchronize
BUG FIX:  #516498 - Installing Stepmania Deletes records
BUG FIX:  #517003 - Joystick mappings not flushed
BUG FIX:  Multiple NEXT presses not allowed on Caution screen
NEW FEATURE:  Skip Caution toggle
NEW FEATURE:  Announcer toggle
BUG FIX:  Arrows no longer jump back to time = 0 on fail or cleared
BUG FIX:  #523621 - Turning on effects in Double (1.62)
BUG FIX:  #521941 - Using pad to select songs acts weird

------------------------Version 1.62-------------------------
CHANGE:  Can now press NEXT to skip Caution screen.
BUG FIX:  Crash at "No music" prompt.
NEW FEATURE:  Synchronize while dancing with '+', '-', "PgUp", "pgDn"
BUG FIX:  DWI parsing no longer crashes on line with only a space

------------------------Version 1.61-------------------------
BUG FIX:  Players' Best crowns not displaying.
BUG FIX:  #506751 - "Assist" Ticks off-beat
BUG FIX:  Crash switching sort
NEW FEATURE:  Synchronize mode!  Can write BMS and DWI formats.

------------------------Version 1.60-------------------------
BUG FIX:  Inefficient use of memory in Select Music screen
OPTIMIZATION:  No noticable loading time in Select Music
CHANGE:  New grade graphics (thanks Parasyte!)
BUG FIX: 511184 - Battle mode not working
BUG FIX: 510557 - Result *** overlays result letter
BUG FIX: 510556 - Sound stops at Options Screen
BUG FIX: NG percentage is "-1%" if no Hold Steps
BUG FIX: #507861 - Multiple Enter on Song Wheel
BUG FIX: #507391 - Song Options menu fade errors
BUG FIX: #506820 - Select Style Versus Images
BUG FIX: #506746 - Difficulty Mix-Ups in Song Select
BUG FIX: #506597 - Empty Sort Crash
BUG FIX: #511543/#511186 - Visualizations not shown correctly

------------------------Version 1.59-------------------------
NEW FEATURE: More texture loading hints (DITHER, NO_MIP_MAPS) for increased quality and faster loading time.
BUG FIX: #506274 Crash changing song after sorting.
BUG FIX: #506343 TextureColor=32 Auto-Reset
CHANGE: SMLauncher integrated with the main StepMania executable.  Now the pretty error screen will appear automatically.
NEW FEATURE:  Added Low-Medium-High detail prompt to installer.
NEW FEATURE: Song sort order is now rememebered.
OPTIMIZATION:  Instantaneous Song Sort Order switch.
OPTIMIZATION:  Instantaneous Song Group expand/collapse.
CHANGE: Song Sort groups that begin with a number now appear under the "NUM" group.
BUG FIX: "Player's Best" is now sorted by num times played decending
BUG FIX: #506835 - PREV from Song Select Fade Glitch
BUG FIX: #506827 - Song Selected Graphic Errors

------------------------Version 1.58-------------------------
NEW FEATURE: SMLauncher generates friendly error messages.
CHAGNE: Removed "NG" from Results screen.
BUG FIX: Statistics are now working, honest!
BUG FIX: 511544 - Grade Results Sometimes pops up NO DATA
OPTIMIZATION: Improved animation of and optimized drawing of rolling score display.
OPTIMIZATION: Toned down motion blur effect in "Failed" for increased performance.
BUG FIX: Fixed matrix stack bug that caused some shadows not to draw
OPTIMIZATION: Major optimization of BitmapText.  Now about 2x faster.
NEW FEATURE: F4-F8: Toggle Fullscreen, Change Resolution, Change Display Color, Change Texture Color, Toggle Statistics
NEW FEATURE: Remebers last selected Steps
NEW FEATURE: Stage number saved to .ini file.
BUG FIX: Default key buttons don't work if .ini file not present on startup
CHANGE: Hold Arrows are now drawn last to first, like in DDR MAX.
NEW FEATURE: Hold Arrows allow you to not hold the button for a second without NG'ing them.
CHANGE: Regular Arrows on the same line as a Hold Arrow now appear green.
NEW FEATURE: Hold Arrows now turn gray player doesn't hold the button.
BUG FIX: #510980 Danger is a static box, not text

------------------------Version 1.57-------------------------
NEW FEATURE: smzip package installer and manager
CHANGE: Installer now uninstalls previous version.
BUG FIX: Package Manager now converts all non-HTTP-friendly characters to '_' when exporting.
CHANGE: Much better texture loading algorithm.  Texture quality and loading time of non-power of 2 texture improved greatly.
CHANGE: New 5th Mix theme graphics.  Thanks thingmabobr!
BUG FIX: #507170 Speed Problem (Got rid of "pauses" caused by buffered writing to log)
BUG FIX: #506309 Installer doesn't overwrite files
BUG FIX: #511187 old banners are shown diagonal

------------------------Version 1.56-------------------------
BUG FIX: 506314 Fixed "Hall of mirrors" on nVidia cards (actually, just a work around for a driver bug)
CHANGE: Couple and Solo now appear in Select Style
CHANGE: New Title Menu options (but you can choose them yet :-)
BUG FIX: Fixed jibberish at the top of Arial Bold font by shifting all letters up one pixel
BUG FIX: 505896 Menu Problems [unreadable/illegible font?]
BUG FIX: Ugly textures with Voodoo3
BUG FIX: Ignore Joystick D-pad now works again.
BUG FIX: Installer-created icon "Open Songs Folder" now fixed for OSes that had problems.
BUG FIX: BMS parse crash fixed.
BUG FIX: Player2 couldn't press up to select different steps
BUG FIX: Player2's side in StepsSelector no longer has multiple bouncing difficulty icons
BUG FIX: 507380 Multiple FAILED with Esc presses
BUG FIX: 507860 - Esc in No Fail Mode = lock up
CHANGE: Improved response time to user input when change sort in SelectMusic
BUG FIX: Announcer no longer makes dancing comments while FAILED is showing.
OPTIMIZATION: Added texture alpha hints - significantly improved texture quality for 16-bit textures.

------------------------Version 1.55-------------------------
BUG FIX: "4nd Stage" is now "4th Stage"
BUG FIX: 496315 ragescreen.cpp error for 16bit-only cards
BUG FIX: 500342 - wave volume changes to 100% on startup
BUG FIX: Fixed "visualization movie files go here" trying to be loaded as a texture
BUG FIX: Mini banner now tweens on and off screen
BUG FIX: MusicWheel now tweens off screen and Music comment plays
BUG FIX: Stage count not incremented until song is passed
CHANGE: Added longer delay before song start
CHANGE: Assist tick now louder and timing improved
CHANGE: cleaned up debug messages
OPTIMIZATION: Improved texture loading time
CHANGE: Maximize button no longer toggles fullscreen.
CHANGE: MusicWheel response to user input is improved
OPTIMIZATION: Preview in SelectMusic now uses banner instead of background for faster loading
NEW FEATURE: Change resolution, display color depth, texture color depth, and toggle texture filtering
NEW FEATURE: Check at startup time that all required theme elements are present
NEW FEATURE: installer
NEW FEATURE: motion blur "Failed" effect
NEW FEATURE: MusicWheel remembers previously selected song.
BUG FIX: Back from Result screen no longer leads to black screen
CHANGE: All documentation is now in "Docs" folder
CHANGE: New "Danger" animation

------------------------Version 1.50-------------------------
Bug fix: No longer crashes on foreign chars in BMS files. Please report all songs that still cause a crash. 
Change: Added sepatate background movie property to Song. Now status backgrounds will display in SelectMusic if the Song has a movie background. 
Change: Asset directory structure has completely changed in preparation for theme support. 
Optimization: BitmapText is now ~80% faster. 
New Feature: 5th Mix style SelectMusic and auto-generated text banners. Loads 100x faster than the previous SelectSong screen. 
Change: Error messages in Release builds no longer show source file line and number. 
Bug Fix: Fade transition now fades all the way to black (doesn't leave a "hazy" screen). 
Bug Fix: Multiple joysticks now work again. 

-------------------------Version 0.97-------------------------
Bug fix: Songs with same title no longer change positions while scrolling.
Bug fix: Wrong group name for DWI files in the Songs folder.
Bug fix: Fixed a problem that caused the music to become out of sync if many sound effect are played.  This was causing the song to become out of sync near the end.  Note that this change makes the music much more accurate, and some BMS files may be out of sync if they appeared in sync before.
Bug fix: Dancing waits 2 seconds before starting music playback.  This allows some slower systems to catch up.
Bug fix: Little now works correctly.  It is accessible from the PlayerOptions menu.
Optimization: Lower priority for movie decoding, which is a major speed boost on slower computers.
Change: Freeze arrow scoring now matches DDR MAX.  The start of a freeze arrow is judged as other steps are (perfect, great, good, etc).  Missing the start of a freeze will should break your combo, but NG'ing the freeze arrow will not. 
New feature: Basic song statistics tracking.  Times played, top score, top combo, and top grade are shown on SelectSong screen.
New feature: Assist tick, accessible from the SongOptions menu. 
New feature: Music Rate, accessible from the SongOptions menu.  You can speed up or slow down the rate of music playback.  Great for practicing hard step patterns.
New feature: Freeze arrow scores are now shown on the Results screen.
Change: Arrow ghost now appear different depending on the combo length (roughly matches DDR MAX).
Change: Added some animation to the Results screen.
Change: Revamped input mapping to allow assignment and clearing of specific slots.

-------------------------Version 0.96b-------------------------
Change: Removed background movies showing in SelectSong (this was causing crashes).
Bug fix: No longer crashes when loading Max300 from DWI support directory.
Bug fix: Timing is now correct for freeze in Max300.
Bug fix: Freeze arrows now are correct in Mirror/Left/Right/Shuffle.

-------------------------Version 0.96-------------------------
Merry Christmas!
Changed the song directory structure to be backward compatible with 0.90 and 0.91.

-------------------------Version 0.95-------------------------
Bug fix: Sounds files with commas now load
Bug fix: Freezes in Telephone Operator are now correct
Bug fix: Nonsense help messages fixed
Bug fix: D3D init failure now opens browser to correct troubleshooting page
Optimization:  Drawing of freeze arrows is much more efficient
Optimization:  In SelectSong, only one preview graphic is drawn.  This saves fill rate and leaves room for group/sort/BPM text.
Change: DWI files now use the DWI directory structure.  StepMania also automatically crop and rotate the song banner.  BMS files still reside in the Songs folder.
Change: Now hold Next to get to PlayerOptions and SongOptions menus  (Previously, you would hold Next to bypass these menus).
New feautre: FPS counter.  Can toggle from GameOptions.
New feature: Disable joystick d-pad (for DirectPad Pro and NTPad).
New Feature: Song background movies
New Feature: random visualization movies
New feature: song groups by folder
New feature: song sorting by Title, BPM, and Artist
New: Caution screen to hide texture load time
New: Logo appears between screens as a keepalive indicator

-------------------------Version 0.91-------------------------
Bug fix: Good and Boo arrows not being drawn.


-------------------------Version 0.90-------------------------


-------------------------Version 0.50-------------------------

************************************************************
End of document
************************************************************
