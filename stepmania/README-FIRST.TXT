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
14. Background Animations
15. The .CRS File Format
16. Extra Stages
17. Creating an Announcer
18. Creating a Note Skin
19. Creating a Theme
20. How StepMania Loads Textures
21. Building StepMania from CVS Source
22. Version History
23. Thanks

************************************************************
1. About StepMania
************************************************************

StepMania game system is capable of playing many game types.  Currently, it 
supports games similar to Dance Dance Revolution, Pump It Up, and EZ 2 
Dancer.  In the future, it will support games similar to BeatMania, Guitar 
Freaks, DrumMania, Para Para Paradise and more.

StepMania is a rhythm game.  Arrows pointing in different directions scroll 
up from the bottom of the screen, and the player must hit the corresponding 
direction in time to the music.  The game can be played with your hands on 
the keyboard, but the real fun comes when the arrows are registered by dancing 
on a sensor-laden mat.  The game is very similar to Konami's Dance Dance 
Revolution and Andamiro's Pump It Up games, but while those play only their 
own songs, StepMania can play any song that includes a step file.

In Stepmania most popular mode, "dance", the player uses four arrows - up, 
down, left, and right - , similar to Konami's Dance Dance Revolution series 
of games.  In "pump" mode, the player must deal with and five direction of 
steps - ala Andamiro's Pump it Up games.  

Later versions of StepMania will support steps similar to Amuseworld's 
EZ2Dancer games, as well as any that there is a sizable demand for.


************************************************************
2. Minimum Requirements
************************************************************

* Windows 98, Windows ME, Windows 2000, or Windows XP
* Pentium II, Pentium III, Celeron, Athlon, or compatible processor, 266MHz 
  minimum (400 MHz recommended)
* 64 MB of RAM
* Video card that supports High Color (16-bit color) and has 16MB video RAM
* DirectX 8.1 compatible video card with drivers
* DirectX 8.1 compatible sound card with drivers
* DivX 5 video codec (for movie visualizations)


************************************************************
3. DirectX
************************************************************

DirectX 8.1 is required before installing StepMania. If DirectX 8.1 is not 
installed on your computer, the StepMania installer will display a warning 
and abort the installation.


************************************************************
4. Installation
************************************************************

To install StepMania, download the file stepmania3.exe to a known location 
on your hard drive, then double-click the file icon to begin the installation.
The installer automatically remove an older version of the program if one is 
present. The default installation directory is 'C:\Program Files\StepMania'.


************************************************************
5. Controls
************************************************************

Use the "Config Key/Joy" option from the Title Menu to configure your 
keyboad, dance pad, or other game controller.

The following is a list of special keys.

Any time:
  * F4 = toggle fullscreen
  * F5 = toggle detail
  * Tab = increase game speed 4x (useful for moving through menus quickly)
  * LShift = decrease game speed to 1/4x

In menus:
  * Arrow keys navigate menus
  * Enter = Start, Escape = Back

In gameplay:
  * F8  = toggle AutoPlay
  * F9  = decrease offset
  * F10 = increase offset
  * F11 = decrease BPM of current segment
  * F12 = increase BPM of current segment
  * Hold Escape = abort playing


************************************************************
6. Playing the Game
************************************************************

This section primarily covers StepMania's most popular game type, "dance".
The various games supported by StepMania are similar, and much of this 
information applies to other games too.

Title Menu screen: At the title screen are a series of options.

  * Game Start:
		Begin playing the game and go to the Style Select screen.  Make sure 
		you've configured your keyboard or dance pad before chosing this 
		option.
  * Switch Game:  
		Allows you to chose between the various games supported by StepMania 
		(dance, pump, ez2, etc).
  * Config Key/Joy:  
		This is where you go to assign key presses or pad presses.  You'll 
		want to use this screen before playing.
  * Game Options: 
		This is you can adjust properties of the game, like difficulty
		and number of stages.
  * Game Options: 
		This is you can adjust graphic detail and other settings that 
		the performance of the game.
  * Appearance Options: 
		Here, you can customize the appearance of StepMania by chosing the 
		announcer, theme, and note skin.
  * Edit/Record/Synch: 
		In this mode, you can edit songs and step patterns, or create your
		own step patterns from scratch.
  * Exit: 
		Exits the program.
  * At the upper-left corner is the total number of songs found when 
		StepMania perused your songs foler on startup.
  * At the upper-right is your version of StepMania.


Config Key/Joy screen:

	This menu allows you to map up to 2 keyboard/joystick buttons to each 
	function in the game.  The third column is the default key mapping, and 
	cannot be changed.  However, you can override a default keyboard key by 
	simply assigning it to another function.  The mappings for player 1 are 
	on the left half of the screen, and the mapping for player 2 are on right 
	half.


Appearance Options screen:
	
  * Display: 
		Toggles between Fullscreen mode, in which StepMania is the only 
		program visible and takes up the entire screen, and Windowed mode, 
		in which StepMania runs in a window and can be moved about on your 
		desktop and minimized or maximized, much like any other window.  
		Many users have found that one mode or the other is more efficient 
		and produces a smoother game.  This varies depending on memory and 
		video functionality.
  * Display Res: 
		This switches between various standard resolutions of the screen.  
		A higher number of pixels implies a sharper image, but requires more 
		computer power.
  * Texture Res: 
		Adjusts the resolution (pixel density) of video textures used in the 
		game.  More detail requires a more powerful computer.
  * Refresh Rate: 
		Sets the number of times that the screen will be redrawn in 1 second.
		Note that this setting will only have an effect in Fullscreen mode.
		'Default' uses the system default refresh rate (60Hz on most computers).
  * Ignore JoyAxes: 
		Many dance mat drivers (if you're using a dance mat) map the arrows 
		as directions on a joystick axis.  A joystick cannot, logically, be 
		in more than one place at a time, such as left but also right.  In 
		the game, it is possible to hit two directions at once (in fact, 
		required at times) and switching this setting to 'on' will make 
		StepMania ignore the joystick requirement so that your dance mat 
		will correctly register two arrows at once.  See section in the README
		"Connecting Dance Pads" for more information on what adapters to use.
  * Menu Buttons: 
		Toggles whether or not the menus will respond to the gameplay buttons.  
		If not, dedicated buttons for menu navigation will be required.
  * Show FPS: 
		Displays the current frames-per-second in the upper right side of the 
		screen.
  * BG Mode: 
		This controls the state of the background behind the arrows.
		OFF - Only the background graphic of the song is displayed.
		ANIMATIONS - Along with the background graphic, animated graphics 
			(found in the BGAnimations folder) will dance around behind the 
			arrows.  Keep in mind that the animation files are small graphics, 
			and must be loaded on the fly.  On slower computers, this will 
			cause jumpiness during the song.  If this is a problem, try 
			changing to Visualizations or even off.
		VISUALIZATIONS - Along with the background graphic, movies (found in 
			the Visualizations folder) will be played.  The default StepMania 
			Visualizations pack (available at www.stepmania.com) contains 
			swirling and twisting effects, but in theory any AVI or MPEG video 
			can be used.  You must have the DivX codec installed on your 
			computer for this to work.  Keep in mind that DivX decoding is 
			processor-intensive, and will decrease the framerate.  Slower 
			computers will have a hard time playing StepMania AND decoding 
			DivX backgrounds.
		RANDOM MOVIES - Movie files from the "RandomMovies" folder will be
			played as the background for the song.
		Sample animations, visualizations, and random movies can be found at 
		www.stepmania.com.
  * BG Brightness:
		Adjusts the brightness level of the backgrounds.  Since the 
		backgrounds and the arrows are both brightly colored, darkening the 
		backgrounds makes the arrows more visible.
  * Menu Timer: 
		Toggles whether or not StepMania will place a time limit on using any 
		given menu.  This limit is usually in the upper-right corner of the 
		screen.
  * Event Mode: 
		If set to ON, there is no limit to the number of songs played.  The 
		player goes back to the Select Music screen even after failing a song, 
		and continues to do so until they press Esc to move back a menu.  If 
		set to OFF, the game will end after a number of songs specified in 
		'Arcade stages', the next option down.
  * Arcade Stages: 
		If Event Mode is OFF, this is the number of songs that can 
		successfully be cleared before the game over screen is displayed.
  * Judge Difficulty: 
		How accurate the player must be in stepping to get a better score for 
		the step.  A higher number means they must be closer to the actual 
		timing of the arrow.  Those of you who try to get all 'perfect' steps 
		will have a monster of a time with this setting all the way up.
  * Life Difficulty:
		How difficult it is to keep the life meter at the top of the screen 
		up.  The higher this number, the less it will increase for precise 
		steps, and the more it will decrease for poorly timed steps.
  * Auto Play:
		The game will play as normal, but will hit all the arrows of its own 
		accord, without the player's input.  It also will get a perfect on 
		every step.  This will be saved as a high score for the song.


Appearance Options Menu:

  * Announcer: 
		Select from the installed Announcer packs.  Get more at 
		www.stepmania.com.
  * Theme:
		Select from installed Theme packs.  Themes change the look of Stepmania.
		Get more at www.stepmania.com.
  * Note Skin:
		Change the appearance of the arrows in the game by selecting from the 
		available packs.  
	Announcer, theme, and note skin packs can be dowloaded from 
	www.stepmania.com.


Song Options Menu

Upon selecting a song, the words 'Press NEXT for options' will appear on the screen.  If you press the 'next' button (enter by default) you will come to the Song Options menu, which contains a variety of settings for changing the way the song and arrows are played.

Speed: The stream of arrows can be stretched out so that it comes faster, but the arrows are more spaced out.  It can be left at 1x (normal speed) or stretched by multiples of .5, .75, 1.5, 2, 3, 5, or even 8 times the original speed.

Effect: Each of the effects changes the arrows themselves in some way, usually in such a way as to make them more obscure.  Consider these challenege modes.

Off - No effect, normal arrow scrolling.
Boost - Also known as 'parabolic' in some circles, this makes the arrows start up slowly and accelerate towards the top.
Wave - Arrows alternately speed up and slow down.
Drunk - Arrows sway back and forth as they scroll.
Dizzy - Arrows spin as they scroll.
Space - Similar to the scrolling effect in the Star Wars intros, arrows scroll away from the player.
Mini - Arrows are really small.

Appear: These change the visibility of the arrows.

Visible - Normal arrow visibility.
Hidden - Arrows disappear before they reach the top.
Sudden - Arrows are invisible until close to the top.
Stealth - Arrows are completely invisible.  The only way to get them is to either memorize the song, watch another player perform the same song at the same time as you on the same difficulty setting, or be really really lucky.

Turn: These rotate the arrows.  The arrows will show up on screen in their rotated positions.

Off - Normal arrows, unchanged from how they are in the step file
Mirror - Steps are rotated 180 degrees.  Up becomes a down.
Left  - Steps are rotated 90 degrees left.  Up becomes a left.
Right - Steps are rotated 90 degrees right.  Up becomes a right.
Shuffle - All steps in the step file are randomly shuffled.

Little: Removes any arrows that aren't on the quarter beats.

Scroll: BY reversing the scroll, the arrows will scroll from the top down instead of from the bottom up.

Color: Changes the arrow colors.

Vivid - Arrows cycle colors, and off-beat arrows are at different parts of the cycle so as to be easily identifiable.
Note - Off-beat arrows are a different color from on-beat arrows, but colors do not cycle.  
Flat - Off-beat arrows are the same color as on-beat arrows
Plain - Same as flat, with the addition that notes do not cycle colors.

Holds: If this is set to OFF, there will only be a normal arrow in place of any hold arrows that may appear.  Some players don't like hold arrows.

Dark: This removes the grey arrows from the top of the screen, so that the player can't use them as a reference as to when to hit the proper arrow.

Life Type: Switch between BAR, where there is a meter that can be increased with accurate steps and decreased with poor steps, and BATTERY, where the player gets a set number of batteries, similar to 'lives' in most games, which are removed as they player misses steps.

Bar Drain: Functional only if Life Type is set to BAR.  Switch between three ways the life meter can function.

Normal - Perfect and Great steps increase the bar, any others decrease it.
No Recover: Same as Normal but it can only go down, not up.  Perfects and Greats leave it in place.
Sudden Death - Any step score other than Great or Perfect will automatically fail.

Bat. Lives: Functional only when Life Type is set to BATTERY.  This sets the number of batteries the player gets.

Fail: If you fail by missing enough steps, this is when the song ends.

Arcade - Song will end as soon as you fail it.
End of Song - As the name implies, you will not fail out until the music end.  You miss every step and the song will keep playing until it's finished.
Off - You cannot fail the song.

Assist: Makes a 'clap' sound every time you are supposed to hit an arrow, to help the player acquaint his or herself with the song's rhythmic structure.

Rate: This is the percent of normal speed that the song plays at.  Any song can be speeded up or slowed down as much as 30%.  This setting is not to correct a song that is misaligned to its steps, as the steps are also sped up or slowed down at the same rate that the song is.



Select Style screen:  This is the screen where you choose your style of play. 

  * Single:
		One player, playing on one four-panel pad.  Second player need not apply.
  * Versus:
		Two players each with their own respective 4-panel pad and set of arrows.  
		Each player can choose their own difficulty, and each has their own set 
		of arrows to follow.  At the end of the song each player will recieve a 
		separate grade.  There is no interaction between players other than 
		comparing grades and, if you have dance mats, the possibility of crashing 
		into each other during play :-)
  * Double:
		Same as single, except that one player uses both player's pads, and must 
		contend with 8 arrows now instead of only four.  The step listings for 
		doubles mode are separate from single or versus.  Only files containing 
		such a set of steps will be displayed.
  * Couple:
		Very similar to versus, except that each player has a different set of 
		steps, and each player must choose the same skill level.  Often stepfile 
		authors use this mode to choreograph a patern between the two players.
  * Solo:
		One player plays on six panels.  Like double, this uses a different step 
		chart than single or versus or double does, and only files containing 
		that set of steps will be displayed.


************************************************************
6. How StepMania Loads Songs
************************************************************

All song files for a specific song (data file, music file, banner, background) 
must reside in a single folder.  This folder must have the following path:

   Songs/<group folder>/<song folder>/

For example, the song B4U may consist of the following files:

   Songs/Dance Dance Revolution 4th Mix/B4U/b4u.dwi
   Songs/Dance Dance Revolution 4th Mix/B4U/b4u.mp3
   Songs/Dance Dance Revolution 4th Mix/B4U/b4u-banner.png
   Songs/Dance Dance Revolution 4th Mix/B4U/b4u.png
   Songs/Dance Dance Revolution 4th Mix/B4U/b4u-movie.avi

StepMania supports the SM, DWI, BMS, and KSF song file formats.

If a song file does not specify a valid banner file, StepMania first looks 
for a graphic in the song folder that contains the letters "banner".  If that
search fails, StepMania will guess which image is most appropriate.

If a song file does not specify a valid background file, StepMania first looks 
for a graphic in the song folder that contains the letters "bg" or "background".
If that search fails, StepMania will guess which image is most appropriate.

If a song file does not specify a valid CDTitle file, StepMania will look
for a graphic in the song folder that contains the letters "cdtitle".  If that 
search fails, StepMania will guess which image is most appropriate.

Also, for compatibility with DWI, if can reading CDTitles from the .\CDTitles 
folder.  Song makers are not encouraged to not use this feature and instead 
should put the CDTitle graphic in the song folder.  Having a separate CDTitles 
folder breaks the principle that all files needed to play a song should reside 
in the song folder.

If a song file does not specify a valid movie background file, StepMania will 
look for a movie in the song folder that contains the letters "movie".  If 
that search fails, StepMania will use the first movie in the song folder.


************************************************************
7. Connecting Dance Pads
************************************************************

There are several devices that allow you to connect PlayStation dance pads to 
your PC.

* Not recommended:  generic PlayStation->USB converter 

Using a generic PlayStation->USB converter is not recommended to play any 
dancing game on the PC.  The vast majority of these converters are not capable 
of registering simultaneous presses of Left+Right or Up+Down.  This 
restriction will cause the player to miss many notes where two buttons must 
be hit simultaneously.

* Highly Recommended:  Special PlayStation->USB adaptor

There is just one brand of PlayStation->USB adaptor that functions perfectly 
with StepMania.  These adaptors are available for purchase at the StepMania 
web site (www.stepmania.com) and through some importers (www.liksang.com).  
The adaptor is made by "Boom", and it small and black with a detachable USB 
cable.  These adaptors cost about $15 and allow you to attach one dance pad 
per adaptor.  The primary advantage of these adaptors is that they do not 
require special drivers and require no configuring.

* Semi-Recommended:  PlayStation->Parallel adaptor

PlayStation->Parallel converters do not have the simultaneous button problem 
that USB converters do.  The disadvantage of these adaptors is that home-made 
drivers are required to use these devices.  These drivers are often finicky 
and incompatible with some sound cards.  PlayStation->Parallel adaptors can 
be purchased from the StepMania web site or any importer.

For Windows 98 and Windows Me users, the recommended driver is "DirectPad Pro 
5.0 for Win9x".  For Windows 2000 and Windows XP, the recommended drivers are 
NTPad XP 1.x.  Both of these drivers can be downloaded from Aldos web site: 
http://www.aldostools.com/dpad.html


************************************************************
8. Compatibility Issues
************************************************************

Some video card/driver combinations have visual errors when running StepMania. 
If you experience any visual errors, please visit your video card manufacturer's 
web site and download the latest Direct3D 8.1 drivers. Below is a list of known 
issues that are thought to be video card driver bugs:

* S3 Savage family cards (e.g. Diamond Stealth)
Symptom: The right edge of some graphics appears "cut off".
Explanation: Drivers do not properly handle textures >= 512x512, resulting in 
cropped textures. Turn the texture detail option to 256 to force textures to 
a smaller size.

* SiS cards 
Symptom: Garbled fonts.
Explanation: Drivers have errors in handling texture coordinates. Only known 
solution is to upgrade drivers.

* Any card older than two years
Symptom: Various graphical glitches.
Explanation: These problems are most often caused by buggy video card drivers
bugs.  Please upgrade to the latest drivers, which will hopefully fix the 
errors.


************************************************************
9. FAQ / Troubleshooting
************************************************************

* The error "Cannot find Dinput8.dll" appears when launching the game.

You need to download DirectX 8.1.  http://www.microsoft.com/directx

* StepMania reports "Direct3DCreate failed".

You need to download DirectX 8.1.  http://www.microsoft.com/directx

* StepMania reports "Direct3DCreateDevice failed".

This error means that your video card or drivers do not support Direct3D 8.  
Download the latest drivers for your video card.  If your video card is older 
than 2 or 3 years, Direct3D 8 drivers may not be available.  Sorry.

* Music rates of over 100% do not work on my computer. Why?

Some very old sound cards do not support sound playback rates that are greater 
than the original sample rate.  This problem is likely to be corrected by newer 
sound card drivers.

* I’m looking to purchase a video card.  What works well with StepMania?

Just about any video card manufactured in the last year will run StepMania 
flawlessly at 60 frames per second.  I recommend any card with an nVidia 
GeForce chip or an ATI Radeon chip.

* How can I improve performance with my existing video card

1. Install the latest drivers for your video card.
2. Press F5 in the game to toggle between high detail and low detail modes.
3. Tweak the Display Resolution and Texture Resolution setting in the GameOptions 
   screen.

* Chris, will you tell me when the next version will be released?

I'd rather not going to say.  Sorry.  It's gotten me in trouble before.

* Why won't you tell me?  I promise I won't tell anyone else.

The reason I won't tell is because: a) I don't know yet, and b) I don't want 
to make  anyone angry if it takes longer than I expect (it usually does).


************************************************************
10. StepMania Packages - .smzip
************************************************************

The StepMania package format was created to make the distribution of songs and 
other add-ons very easy.  StepMania package files have the extension '.smzip' 
and can be installed by double-clicking the .smzip file.

A StepMania package is 'installed' by extracting all files in the package to the 
StepMania program directory. This allows songs, courses, themes, and 
visualizations to all be installed by the Package Manager.

The file format of an .smzip file is actually the PK-Zip standard.  This means 
you can rename any .smzip file to have the extension '.zip', and then open the 
file in any compression package (e.g. WinZip).

The StepMania Package Manager (smpackage.exe) can also create song packages 
for you.  Simply launch the Package Manager 
(Start Menu->Programs->StepMania->Package Manager), click the song you want to 
create a package for, click the Export button, and a .smzip package will be 
placed on your desktop.


************************************************************
11. Questions, Bugs, Suggestions, and Help
************************************************************

If you have a question or problem about StepMania, please try asking your 
question to the StepMania message board.  A link is available from the StepMania 
web site - http://www.stepmania.com.

If you have found a bug, please file a bug report into the SourceForge bug 
tracking system (http://sourceforge.net/tracker/?group_id=37892&atid=421366).  
This will allow the developers to track and fix bugs much more efficiently.  

* In a bug report, please specify your video card brand and model, and your OS 
version.
* PLEASE, PLEASE enter your e-mail address on the bug reports. In many cases, 
we need more information from the bug reporter.

Please use SourceForge's feature request system to enter new feature requests 
(http://sourceforge.net/tracker/?atid=421369&group_id=37892&func=browse).  Also, 
please discuss feature ideas on the StepMania message board or the StepMania 
developers list (stepmania-devs@sourceforge.net).  

If you would like to help with development of StepMania, please send an email 
to the StepMania developers list (stepmania-devs@sourceforge.net) or to Chris 
(chris@videogamers.com).


************************************************************
12. The StepMania editor
************************************************************

The StepMania editor allows you to edit, record, synchronize, and play back 
note patterns.  Current, it support saving only in the .SM format.  If you save 
a song that was originally in DWI or BMS format, the old DWI and BMS files will 
be renamed to have ".old" appended to avoid conflicts when the song is next 
loaded.  All of the commands are listed along the left side of the screen.


************************************************************
13. The .SM file format
************************************************************

The .SM song file format was created to be one file format that supports all 
game types that StepMania can play (dance, pump, beat, guitar, etc).  The .SM 
file syntax is identical to DWI except for some different tags and meanings.  
These are a list of all supported tags.

All images can be in png, gif, jpg, or bmp format.  All sounds can be in ogg, 
mp3, or wav format.


#TITLE:...;    - The "main title" of the song.
#SUBTITLE:...; - This text will appear underneath the main title of the song 
on the Select Music screen.  e.g. "-Dirty Mix-", "(remix)".
#ARTIST:...;   - The artist of the song.
#CREDIT:...;   - Give yourself some credit here for creating a wonderful song.
#BPM:...;      - BPM of the music
#BANNER:...;   - The file name of the banner image.  e.g. "b4u-banner.png".  
This image must reside in the song folder.
#BACKGROUND:...;   - The file name of the background image.  e.g. "b4u-bg.png". 
This image must reside in the song folder.
#CDTITLE:...;   - The file name of the spinning CD logo.  e.g. "b4u-cdtitle.png".  
This image must reside in the song folder.
#MOVIEBACKGROUND:...;   - The file name of the a background movie.  e.g. "b4u-movie.avi".  
This movie must reside in the song folder.
#MUSIC:...;   - The file name of the music file.  e.g. "b4u.mp3".  This image 
must reside in the song folder.
#MUSICBYTES:...;   - The size in bytes of the music file that this song was last 
synchronized with.  A "Song may be out of synch" warning will be displayed if 
the value of this tag does not match the actual size of the music file.
#OFFSET:...;   - The time in seconds at which beat 0 occurs in the music.  This 
is specified as a floating point value.  e.g. "2.34".
#SAMPLESTART:...;   - The time in seconds to start the music sample that plays 
on the Select Music screen.  This is specified as a floating point value.  e.g. 
"32.34".
#SAMPLELENGTH:...;   - The time in seconds let the sample music play after 
starting.  This is specified as a floating point value.  e.g. "32.34".  Note 
that in the last 1 second of playing the music will fade out.
#BPMS:...; - A value of the format "beat=bpm".  Indicates that at 'beat', the 
speed of the arrows will change to "bpm".  Both of these values are specified 
as floating point values.  You must specifiy a BPM value for beat 0.  Multiple 
BPMs can be given by separating them with commas.  e.g. "0=160,120=80".  
#STOPS:...;   - A value of the format "beat=sec".  Indicates that at 'beat', 
the motion of the arrows should stop for "sec" seconds.  Both of these values 
are specified as floating point values.  Multiple stops can be given by 
separating them with commas.  e.g. "60=2.23,80=1.12".
#ANIMATIONS:...;   - A value of the format "beat=anim name".  Indicates that at 'beat', 
the background animation should change to the animation names 'anim name'.  'beat'
is a floating point value and 'anim name' is a string.  Different animation change 
are separated by commas.  e.g. "60=falling,80=flower".


Each pattern of steps for different modes have the same basic format:

#NOTES:<NotesType>:<Description>:<DifficultyClass>:<DifficultyMeter>:<RadarValues>:<NoteData>;

NotesType:  Must be one of the currently supported types in StepMania: 
"dance-single", "dance-double", "dance-couple", "dance-solo", "pump-single", 
"pump-double".

Description:  This will be displayed on the gameplay screen.  This can be any 
text, but is most commonly "Basic", "Another", "Trick", "Standard", "SSR", 
"Maniac", "Heavy", or "Challenge".

DifficultyClass:  This value must be "easy", "medium", or "hard".  These values 
correspond the levels of difficulty on the Select Difficulty screen.

DifficultyMeter:  The difficulty of these notes as a bar rating.  The value must 
be an integer between 1 and 10.

RadarValues:  The values that will be displayed on the Groove Radar as floats 
between 0.00 and 1.00.  The 5 values must be separated by commas, and are 
specified in the order: "stream,voltage,air,freeze,chaos".  For MAX300, these 
values are "0.95,0.98,0.22,0.12,0.13".  These values will automatically be 
written when you save the song in the StepMania editor, but you may edit the 
.SM file afterward to tweak them.

NoteData:  This value requires a longer explanation.  The format is pretty 
similar to that used in KSF files.

Each note is represented by a character:

1 = a regular "tap note"
2 = beginning of a "hold note"
3 = end of a "hold note"
a-z,A-z = reserved for game types that have sounds associated with notes

Notes are grouped into rows.  The number of notes in a row depends on the 
NotesType field.

dance-single = 4 notes/row
dance-double = 8 notes/row
dance-couple = 8 notes/row
dance-solo = 6 notes/row
pump-single = 5 notes/row
pump-double = 10 notes/row

For example, a note row for a dance-single pattern may look like "1001" (press 
Left and Right simultaneously.  A note row for a dance-double pattern may look 
like "10010000" (press Left and Right simultaneously on the left pad).

Note rows are grouped into measures.  The number of note rows in a measure will 
determine the time value of each note.  For example, if there are 4 note rows
in a measure, each note will be treated as a quarter note.  If there are 8 notes 
rows in a measure, each note will be treated as a eighth note.  If there are 12 
notes rows in a measure, each note will be treated as a triplet note.  Measures 
are separated by a comma.

Example:

   // measure 1
2010
0000
0100
0000
:  // measure 2
0001
0100
0001
0000
3010
0000
0000
0000
;

************************************************************
14. Background Animations
************************************************************

\BGAnimations\<anim folder>\<layer files>

The BGAnimations folder should contain several folders for individual background animations.  Currently, these background animations cycle in order of their folder names, changing every 4 measures of the song.

In an animation folder, each sprite represents one layer.  Multiple layers can be specified by having multiple sprites.  Layers are drawn from back to front in order of their file name.  Different "layer effects" are specified by adding tokens to the file name.  Here are an explanation of the currently supported tokens:

usesongbg - Use the song's background file instead of this graphics file.  Since this graphics file will not be used, you might want to make this graphic a 1x1 graphic to save space.

add - use additive blending instead of normal blending.

cyclecolor - cycle the color of the layer over the colors of the rainbow

cyclealpha - cycle the alpha channel of the layer over type

startonrandomframe - Start the animation on a random frame.  This is mainly useful for Tile or Particle effects.  

dontanimate - Stay on the first frame of this animation.  This is only useful for layers with multiple frames.

(stretch effects) - These scretch the graphic across the extire screen.
scretchscrollleft,
stretchscrollright,
stretchscrollup,
stretchscrolldown,
       - Scroll the layer in a direction

stretchwater,
stretchbubble,
stretchtwist,
stretchspin,
       - Deform the background sprite with an effect.  Only stretch spin is currently implemented.

(particle effects) - Use the graphic to make individual particles on the screen.
particlesspiralout,
particlesspiralin,
       - Spiral particles away from/toward the center of the screen.
particlesfloatup,
particlesfloatdown,
particlesfloatleft,
particlesfloatright,
       - Scroll the particles across the screen.
particlesbounce,
       - Particles start traveling in random directions and bounce when they hit the edge of the screen.

(tile effects) - Tile the graphics across the screen, forming a grid of graphics.
tilestill,
tilescrollleft,
tilescrollright,
tilescrollup,
tilescrolldown,
       - Scroll the tiles

tileflipx,
tileflipy,
       - Flip the tiles along the X or Y axis.

tilepulse,
       - Tiles zoom in and out.


************************************************************
15. The .CRS File Format
************************************************************

Place holder...

Header section...
#COURSE:...; - name of the course
#LIVES:<0..10>;  - An integer between 0 and 10.  This line is optional.
#REPEAT:<YES|NO>;  - start over after last stage?
#SONG:<group folder>\<song folder>:<description>[:modifiers]; - 
  group folder - The name of the folder that contains the song folder.
  song folder - the name of the folder that contains files for the song
  description - BASIC, TRICK, SSR, MANIAC, HEAVY, etc.
  modifiers - One or more of:
	0.5x, 0.75x, 1.5x, 2.0x, 3.0x, 4.0x, 5.0x, 8.0x
	boost, wave, drunk, dizzy, space, mini
	hidden, sudden, stealth 
	mirror, left, right, shuffle
	little
	reverse
	note, flat, plain
	noholds
	nofreeze
	dark
	norecover, suddendeath, power-drop, death
	0.7xmusic, 0.8xmusic, 0.9xmusic, 1.0xmusic, 1.1xmusic, 
		1.2xmusic, 1.3xmusic, 1.4xmusic, 1.5xmusic


************************************************************
16. Extra Stages
************************************************************

You can manually specify extra stage 1 and 2 by creating a .CRS files in the 
group folder called extra1.crs and extra2.crs.  These courses must contain 
exactly one valid song.  Don't forget to specify modifiers - see the CRS File 
Format section in the readme for more information.  If a CRS file is not found
or is invalid, the program will choose approprite Song/Notes/Options.

************************************************************
17. Creating an Announcer
************************************************************

Announcers folders must reside in the "Announcers" folder inside the 
StepMania program directory (e.g. "Announcers\DDRMAX\").  An announcer folder 
contains several more folders - one for each announcer "trigger".  You can 
place as many .WAV, .MP3, or .OGG sounds as you want in a trigger folder.
When StepMania wants to play a sound for that trigger, it will play a 
random sound file from your trigger folder.

Sound files in a trigger folder can have any name you want.  It's recommended
that you name the sound files according to what is being said (e.g. 
"everybodys waiting for you.mp3").  If there are 0 sound files in a trigger 
folder, or the trigger folder is missing, then no sound will be played for 
that trigger.

************************************************************
18. Creating a Note Skin
************************************************************

Note skins allow you to customize the way notes appear.  Note skins must 
reside in the folder "Skins\(Game Name)\(Note Skin Name)".  Note skins are 
specific to a particular game (dance, pump, etc).  In the note skin folder,
you must implement the folling files for each note:

<NoteName> Receptor (NxN).<png|gif|jpg|bmp>
<NoteName> Note color part (NxN).<png|gif|jpg|bmp>
<NoteName> Note gray part (NxN).<png|gif|jpg|bmp>
<NoteName> Hold Explosion (NxN).<png|gif|jpg|bmp>
<NoteName> Tap Explosion Bright (NxN).<png|gif|jpg|bmp>
<NoteName> Tap Explosion Dim (NxN).<png|gif|jpg|bmp>
<NoteName> Hold.colors
<NoteName> Tap.colors

"<NoteName>" must be the name of the note (e.g. "left", "upright", "snare").
Look at the example note skins included with StepMania for a complete list
for each game.

"NxN" is an optional string that specifies the dimensions of the graphic.  
For example, if there are 6 frames, 3 wide 2 high, in your color part 
animation, then you might use the file name "Up Note color part 3x2.png".

For more information on about graphics loading, see the section "How 
StepMania Loads Textures".

************************************************************
19. Creating a Theme
************************************************************

Themes must reside in the Themes folder in the StepMania program directory.
Inside each theme folder, there are up to three directories: "graphics",
"sounds", and "fonts".

Graphics may be .sprites or any file format described in the section "How 
StepMania Loads Textures".  Sounds can be in .mp3, .ogg, or .wav.  Fonts
must use the .font file format.

When StepMania looks to load a font element (graphic, sound, font), it 
first looks in the current theme folder for the element.  If that element
cannot be found, it will fall back to using the element in the theme named
"default".  "default" is sort of a base theme and should not be modified.

This system allows user-made themes to override as little or as many theme
elements as they want.  For example, you could create a theme that does
nothing more than override the title menu logo.  Such a theme would contain 
only one folder ("Graphics") and one file ("Graphics\title menu logo.png").


> What's in a .font file?

Example:

[Font]
Texture=Score Numbers 7x2.png
Widths=5,8,10,17,15,18,15,6,9,9,8,14,5,9
Characters=01234 :56789~.
CapitalsOnly=0
DrawExtraPercent=0.1

Texure: File name of texture file.  Must be in same directory as .font file.
  Font graphics can be generated by the program Bitmap Font Buider
  (http://www.lmnopc.com/bitmapfontbuilder/).
Width:  The width in pixels of the character in each frame of the texture.
  Characters are specified in a left-to-right, top-to-bottom ordering.
Characters:  The characters corresponding to each frame in the texture.
  Characters are specified in a left-to-right, top-to-bottom ordering.
CapitalsOnly:  0=false, 1=true.  Use "1" if this font implements only 
  capital letters.
DrawExtraPercent:  A number between 0.0 and 1.0 specifying the maximum 
  amount in percentage of the frame width that a character can hang over
  its width.  Many italic fonts hang a actually a little bit wider than 
  base of the characer because of skewing.  For these types of font, the
  DrawExtraPercent should be > 0.  This value takes some playing around with.

************************************************************
20. How StepMania Loads Textures
************************************************************

> What are textures?

"Texture" is simply another name for graphics file.  The name "texture"
is more common when taking about 3D graphics.  This document uses "texture"
and "graphic" interchangably.

> What types of graphics can StepMania read:

StepMania can use any of the following graphics formats as textures:
png, gif, jpg, bmp, avi, mpg, mpeg.  StepMania looks for textures on disk
only by looking at file name minus the extension.  For example, when 
StepMania wants to load the graphic "title menu logo", it will first look
for "title menu logo.png", then "title menu logo.gif", ... then "title 
menu logo.mpeg".


> How to you specify multiple frames in an animation?

Simply add the string "(NxN)".  Immediately before the "." that separates
the main file name and the extension.  For example, suppose there is a file
called "title menu logo.png" that you would like to turn into an animation.
Now, you create an graphic file with 9 frames of animation - 3 frames wide
and 3 frames high.  Name this file, "title menu logo 3x3.png", and stepmania
will play the graphic as an animation with frames in the order left-to-right,
top-to-bottom, with 0.20 seconds separating each frame.

> What are .sprite files?

.sprite files are a way to have even more precise control over how your 
graphics animate.  Each sprite has multiple "states".  For each state, you
may specify: 1) What frame of the graphic to show, and 2) how long to show
this state before moving to the next state.

Let's look at an example .sprite file:

[Sprite]
Texture=dancer p1 8x4.png
Frame0000=0
Delay0000=0.05
Frame0001=1
Delay0001=0.05
Frame0002=2

Texture is the name of the texure file to use.  This must reside in the same
directory as the .sprite file.  If your texture has multiple frames, be sure
to specify the dimensions in the file name using the convention described 
above.

FrameNNNN is the index of the frame number to use for the first state of the
animation.  Frame may range between 0 and (total number of frames - 1).  
Frame indicies move left-to-right, top-to-bottom over the frames in an image.

DelayNNNN is the time in seconds to show the state.


> My graphics show ugly banding or look washed out.  What can I do?

You can use texture hints to have more precise control over how your graphics
are storred on the graphics card.

All are textures loaded by StepMania are 16-bits.  The default the texture 
format is 4444 (4 bits for R,G,B, 4 bits for alpha), which can cause some 
banding artifacts or a washed-out look.  You can change the texture format by 
adding "hints" to the file name.  Hints:

"no alpha" - This will force a 5650 texture format (5 bits for R, 6 for G, 5 
for B, 0 for alpha).  5650 textures cannot have transparency, but have much 
better color fidelity.
"1 alpha" - This will force a 5551 texture format.  If your graphic has only 
"on or off" alpha (like a GIF file), use this hint for better color fidelity.
"dither" - This will dither an image upon load.  Dithering will reduce banding, 
but can cause transparency artifacts.  

Hints can be placed anywhere in the file name, but are usually placed at the 
end of a file name in parenthesis. For example:

caution (dither, no alpha).png
Arrows Left 1x4 (1 alpha).png

> Why do the graphics look so blurry?

Some 3D graphics cards (the Voodoo3 in particular) cannot handles textures > 
256x256.  If a texture is larger than the card's maximum texture size, the 
image will scaled down internally.  Since the card is using a low-resolution 
version internally, the output, when scalled back to the original image size,
will be blurry.

Whenever possible:
* Keep your images to dimensions smaller or equal to 256x256.
* Make your images have an even number for each dimension (e.g. 286x120 is 
will appear less blurr than 285x119).  The reason behind this has to do with
how 3D accelerators represent screen coordinates with floating point numbers,
not integers.

************************************************************
21. Building StepMania from CVS source
************************************************************

CVS is the repository that manages the StepMania source code.  Our CVS server is 
graciously provided by SourceForge.  You can get the absolute-latest bleeding 
edge updates to StepMania by taking the source from CVS.  Below are instructions 
on how to configure your computer to download source code from CVS and complile 
the source into a working executable.  If you do not have a basic understanding
of how to build a program using Visual C++, then you probably should not attempt
to build StepMania.

* Step 1: Install and configure Visual C++

The StepMania source can currently be built with Microsoft Visual C++ 6.0 or 
Visual C++ .Net.  Installing Visual C++ or Visual Studio with all the default 
options is sufficient.

* Step 2: Install the Direct X 8.1 SDK

The DirectX 8.1 SDK is a free (but large) download from Microsoft (link here).  
Install it using all the default settings.  The SDK installer will configure 
Visual C++ with the correct paths needed to find the DirectX 8.1 libraries and 
headers.

* Step 3: Create a SourceForge account

In order to commit changes to CVS, you'll have to have a SourceForge account.
Create an account at www.sourceforge.net, then e-mail Chris 
(chris@videogamers.com) with your username so he can add you to the project.

* Step 4: Install WinCVS

A CVS client is required to "checkout" the course from the CVS server.  Download 
version 1.2 of WinCVS here - http://www.cvsgui.org 
(or http://sourceforge.net/project/showfiles.php?group_id=10072)

IMPORTANT: Now, open and close WinCVS once.  This will cause WinCVS to create 
registry entries, which is required for the next step...

* Step 5: Use SourceForge Setup to install SSH and configure WinCVS

A small utility called SourceForge Setup will greatly simplify the steps needed 
to configure SSH and WinCVS.  Download SourceForge Setup from 
http://sfsetup.sourceforge.net/.  Use the following settings when prompted by SF 
Setup:

- Unpack SSH directory: c:\ssh
- SSH install direcotory: c:\ssh
- Your home directory: c:\ssh
- Your SourceForge user name: -yourusername-
- Name of the project: stepmania
- The directory where you will be keeping checked out project sources: 
c:\stepmania

IMPORTANT: If you are using Windows 2000 or Windows XP SourceForge Setup will 
not correctly write changes to the c:\autoexec.bat file.  Verify that the 
following lines are present after you run SF Setup:

rem SourceForge setup stuff
SET PATH=%PATH%;c:\ssh
SET HOME=c:\ssh

IMPORTANT: You must restart after running SF Setup so that the new environment 
variables take effect.

* Step 6: Enable SSH access to the cvs machine

Before you can use your account, you must run the following (substituting your 
username and projectname as appropriate) before attempting to use CVS from a new 
user account.  Open a command prompt, and type:

ssh -l username cvs.stepmania.sourceforge.net

Enter your password, and you'll get a message about how you're not supposed to 
be there and the connection will drop. What they don't tell you is that this 
creates your home directory and is required for things to start working.

* Step 7: "Checkout" the source code

- Open up WinCVS again.  
- In the right pane, you should see the folder "stepmania".
- Right click on that folder and choose "Checkout module...".  For the module name
	enter "stepmania".  
- Click OK.  
- If a "chose your home directory" window appears, select "c:\stepmania".
- A minimized console window should appear.  Unminimize it, type in your 
	SourceForge password, and press Enter.
- The source code should begin downloading.  This can take a long time depending 
	on your connection speed.

* Done!  Enjoy staying on the bleeding edge of StepMania releases! 


************************************************************
22. Version History
************************************************************

------------------------CVS after 3.00 beta 6----------------
CHANGE:  A game that doesn't have any note skins installed is now invisible on 
	the Select Game screen.  This can be used to selectively disable a game.
CHANGE:  Graphics and sounds now more compressed more for a smaller download.
CHANGE:  The "Difficulty Banner" (displayed while dancing) is now a graphic
	that can be overwritten by themes.
CHANGE:  An invalid character in a DWI file is now a warning, not a Fatal Error.
BUG FIX:  Fixed "blinky arrows".
CHANGE:  Note Skin format has changed.  Now supports MAX-style holds.
OPTIMIZATION:  Improved font alignment.  Scaled fonts now appear less blurry.
BUG FIX:  Random Movies no longer upside down.
NEW FEATURE:  Extra stage top and bottom bars now reversed
NEW FEATURE:  Most graphical elements can be positioned per theme using 
    metrics.ini in the theme folder.  See the theme section of the readme for 
	more info.
BUG FIX:  Autoplay no longer misses the first step if it lies on beat 0.
NEW FEATURE:  Support for holds notes in KSF.
CHANGE:  KSF title discovery will use song foler name if KSF file contains 
	foreign characters.
BUG FIX:  Stepping before start of a note at beat 0 no longer results in a BAD.
BUG FIX:  Editor now works with dance-couple note patterns
BUG FIX:  Lead-in/lead-out time added to Play and Record in editor.
CHANGE:  Song specific Background Animations removed in favor of a more powerful
	system.
NEW FEATURE:  "Show Danger" toggle in game options to enable/disable
	flashing danger message.
CHANGE:  "Event Mode" option renamed to "Unlimited Arcade Stages".
NEW FEATURE:  Graphic Option "MovieDecodeMS".  Controls how many milliseconds
	the program will sleep each frame to allow the movie background movie
	decoding thread to catch up.  Increase this number if movies are dropping
	frames.  Decrease this number to decrease the priority of movie decoding.
CHANGE:  "SongFolders" renamed to "AdditionalSongFolders".
BUG FIX:  AdditionalSongFolers no longer creates duplicate entries on program
	load.
CHANGE:  RandomMovies do not start playing until the first beat
OPTIMIZATION:  New deferred rendering architecture uses more CPU in order to
	create more efficient operations for the graphics card.

------------------------CVS after 3.00 beta 5----------------
OPTIMIZATION:  Much faster song loading.
BUG FIX:  New hashing algorithm prevents multiple songs from hashing to the same 
     cache file.
BUG FIX:  "Select Group" screen now only displays songs and groups available in
     the current style (instead of all songs).
BUG FIX:  Opening a "(NEW)" sequence in Edit/Record/Synch mode with a style
     over 4 panels (such as dance-solo or ez2-single) now works.
BUG FIX:  Oni life meter finished.
NEW FEATURE:  Theme support.  See readme for more info.
BUG FIX:  Stop times now read correctly again in BMSs.
BUG FIX:  #SOLO steps in DWIs now read correctly.
OPTIMIZATION:  Startup time greatly reduced thanks to hiding the log window and
     SM parsing optimizations
CHANGE:  Display resolution and max texture size sliders re-enabled
NEW FEATURE:  Refresh rate slider
NEW FEATURE:  0.5x, 0.75x scroll speed
NEW FEATURE:  Song rates up to 1.5X
NEW FEATURE:  Effect Mini
CHANGE:  Now hold L&R while pressing start to change sort on Select Music
CHANGE:  Combo display now draws behind notes
BUG FIX:  Stage counter reset after fail (thanks Andy)
BUG FIX:  Roulette chooses from only last preferred group
BUG FIX:  Fail End of Song setting doesn't get stuck at end of song.
CHANGE:  Back button must now be held for 1/2 a second to prevent accidental presses.
BUG FIX:  No more slowdown when song background showing song background (I think).
BUG FIX:  Fixed clipping problems related to scroll speed
BUG FIX:  Hold notes no longer jitter at certain scroll speeds
NEW FEATURE:  During gameplay:  F8 = toggle AutoPlay
NEW FEATURE:  During gameplay:  F9 = decrease offset, F10 = increase offset, 
NEW FEATURE:  During gameplay:  F11 = decrease BPM of current segment, F12 = 
	 increase BPM of current segment.
NEW FEATURE:  After changing Offset or BPM during gameplay, I prompt will appear
	at the end of the stage or course asking if you want to save changes.
	Chosing cancel will revert to the pre-modified values.
CHANGE:  Life meters graphics are now part of the theme system
CHANGE:	 New Song Option:  Choose Oni life battery or normal life meter
BUG FIX:  Life Bar no-recover and sudden death options now work.
BUG FIX:  Select Difficulty arrow no longer messed up on early input
BUG FIX:  Fixed scoring inaccuracies in Oni
NEW FEATURE:  Endless, next to Oni Nonstop
BUG FIX:  Inaccurate earned Chaos radar values fixed
BUG FIX:  Score multiplier now based on note number and not current combo size.
	This change causes higher, but MAX2-accurate scores.
NEW FEATURE:  KSF file reading.  Just drop the KSF folders into a group folder.
NEW FEATURE:  Pump It Up support
NEW FEATURE:  Life Difficulty option - this option determines how much the life 
    meter changes when steps are judged.  This option corresponds to Difficulty in 
	DDR Arcade machines.
CHANGE:  Old difficulty option renamed to Judge Difficulty.
NEW FEATURE:  MenuInput option.  Setting this option to "USE ONLY DEDICATED MENU 
	BUTTONS" will allow the menus to be controlled only by the buttons mapped to
	MenuLeft, MenuRight, etc.  This is useful if you have a controller with 
	buttons that can be dedicated to controlling the menus (like a DDR arcade
	cabinet).  Also, using only dedicated menu buttons will allow you to enter
	codes at the Select Music screen using the dance panels.
NEW FEATURE:  Random background movies.  Put a bunch of movies in RandomMovies and
	change the BackgroundMode setting in Game Options to "RANDOM MOVIES".
CHANGE:  Made folders for DWI-style CDTitles and DWI-style Music.
BUG FIX:  Fixed more HoldNote drawing ugliness.
BUG FIX:  Hidden and Sudden now work with Reverse.
BUG FIX:  Group names are now correct when using the SongFolders option
BUG FIX:  No longer crashes in Select Group with > 15 groups
NEW FEATURE:  PlayerOptions and SongOptions displays on the Select Music screen.
BUG FIX:  Extra stage actually works!  Props to Bruno for all of his help with this.
	You can manually specify extra stage 1 and 2 by creating a .CRS files in the 
	group folder called extra1.crs and extra2.crs.  These courses must contain 
	exactly one valid song.  Don't forget to specify modifiers - see the CRS File 
	Format section in the readme for more information.  If a CRS file is not found
	or is invalid, the program will choose approprite Song/Notes/Options.
BUG FIX:  Browsing past a song that don't have notes for some Difficulties will not 
	change your preferred difficulty.
BUG FIX:  TurnMirror, TurnLeft, and TurnShuffle now work.  TurnRight does the same 
	thing is as TurnLeft temporarily.
NEW FEATURE:  Demonstration plays after sitting idle at title screen.
CHANGE:  Note Skins can now control both the tap tween colors and hold tween colors
	with the files "(ButtonName) Tap.colors" and "(ButtonName) Tap.colors".
CHANGE:  Tap colors now interpolated for smoother color animation.
CHANGE:  Tweaked Drunk for a more interesting effect.

------------------------Version 3.00 beta 4------------------
BUG FIX:  Fixed crash entering Config Joy/Key (duh!)
BUG FIX:  Smarter bannner/backgroung/cdtitle discovery
NEW FEATURE:  Supports reading CDTitles from the .\CDTitles folder (compatible 
     with DWI).  Song makers are not encouraged to use this feature and instead should
     put the CDTitle graphic in the song folder.  Having a separate CDTitles folder 
     breaks the principle that all files needed to play a song should reside in the 
     song folder.
BUG FIX:  Fixed "D3DCreateTextureFromFile" error.  This happend a lot on a Voodoo3 
     and Win98.
BUG FIX:  Fixed "Divide by 0"/"Access Violoation" when loading a song - I think.
OPTIMIZATION:  Faster loading at "Nth Stage" screen.

------------------------Version 3.00 beta 3------------------
BUG FIX:  Fixed crash BemaniRuler's "Connected"
BUG FIX:  #BPMCHANGE in DWI files now read fractional values (BemaniRuler's Naturally)
BUG FIX:  Dizzy works correctly again
BUG FIX:  Subtitle extraction now works for "Dub-I-Dub" and "SP-Trip Machine"
BUG FIX:  No longer crashes on Titles starting with '('
BUG FIX:  Earlier scrolling for songs that have early notes (BMR's A)
BUG FIX:  Inserted blank time for songs that have note right at end of song (AM-3P)
OPTIMIZATION:  Faster loading of gameplay screen
BUG FIX:  Sample music no longer studders on Select Music screen
BUG FIX:  "Shaky" artifacts no longer appear while holding a hold note
CHANGE:  Re-added support for movie visualizations (by popular demand).
NEW FEATURE:  Movie backgrounds.  Either specify with the "#MOVIEBACKGROUND" tag, 
     or the first movie file in the song directory will be used.  Works with
	 BemaniRuler's background movies.  Background movie will not play will not play 
	 if BackgroundMode is set to "OFF".
NEW FEATURE:  Song-specific animations.  Place all animations in a folder called 
     "BGAnimations" within the song folder.  See "13. The .SM file format" and 
     "14. Background Animations" in README-FIRST.txt for details.
CHANGE:  Appearance Options now available from Title Menu.  Here you can choose
     the announcer and note skin.

------------------------Version 3.00 beta 2------------------
BUG FIX:  Fixed crash when loading MAX2 files from ddrei.com
CHANGE:  "JudgeWindow" option name changed to "Difficulty"
CHANGE:  Initial difficulty made easier
BUG FIX:  Song loading now smarter about how it picks banners, backgrounds, 
     and CDTitles.  See readme "6. How StepMania Loads Songs" for details.
BUG FIX:  #BPMS in SM files now read fractional values
NEW FEATURE:  Background brightness (Game Options)
BUG FIX:  BemaniRuler songs (like Pulse) now loaded correctly
BUG FIX:  Stationary arrows don't press unless note is a hit
BUG FIX:  Size of combo text capped to a reasonable level
BUG FIX:  Song background now stretched correctly when used in a background animation
OPTIMIZATION:  Reduced lag while starting music sample (Select Music screen)
NEW FEATURE:  Menu Timer - accessable from Game Options
BUG FIX:  Changes saved in editor now show up immediately (instead of after restart)

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
NEW FEATURE (partially complete):  Nonstop Challenge (Oni)
NEW FEATURE (partially complete):  Pump It Up support
NEW FEATURE (partially complete):  Background Animations (defaut animations go in 
     BGAnimations dir)

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
BUG FIX: Scaled sprite alignment (no more transparent strip along the edge of 
     "danger" static)
NEW FEATURE:  Repeating input
NEW FEATURE:  Select Group menu
NEW FEATURE:  Edit / Record / Playback
NEW FEATURE:  Support for DWI solo notes in DWI reader

------------------------Version 1.65-------------------------
BUG FIX:  Statistics now save and load correctly
BUG FIX:  Better framerate during gameplay

------------------------Version 1.64-------------------------
NEW FEATURE:  Group banners on Select Music screen.  Just put a banner in the group 
     folder.
BUG FIX:  Hacked around a Savage texture loading bug.
CHANGE:  Added "hold NEXT for options" after choosing a song.  Most people don't know 
     about this feature!
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
BUG FIX:  Pixel/Texel alignment bug causing a strip of color along right and bottom 
     edges moving between menus.
BUG FIX:  Graphical glitches on Results screen
BUG FIX:  Hold Steps now look better when Scroll Speed is > 1
BUG FIX:  Wrong Ghost Arrow graphic being used
BUG FIX:  Boost/Scroll Speed interaction is now like MAX
BUG FIX:  DWI's with newlines before semicolon are no longer corrupt when saving 
     using Synchronize
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
NEW FEATURE: More texture loading hints (DITHER, NO_MIP_MAPS) for increased 
     quality and faster loading time.
BUG FIX: #506274 Crash changing song after sorting.
BUG FIX: #506343 TextureColor=32 Auto-Reset
CHANGE: SMLauncher integrated with the main StepMania executable.  Now the 
     pretty error screen will appear automatically.
NEW FEATURE:  Added Low-Medium-High detail prompt to installer.
NEW FEATURE: Song sort order is now rememebered.
OPTIMIZATION:  Instantaneous Song Sort Order switch.
OPTIMIZATION:  Instantaneous Song Group expand/collapse.
CHANGE: Song Sort groups that begin with a number now appear under the "NUM" 
     group.
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
NEW FEATURE: F4-F8: Toggle Fullscreen, Change Resolution, Change Display Color, 
     Change Texture Color, Toggle Statistics
NEW FEATURE: Remebers last selected Steps
NEW FEATURE: Stage number saved to .ini file.
BUG FIX: Default key buttons don't work if .ini file not present on startup
CHANGE: Hold Arrows are now drawn last to first, like in DDR MAX.
NEW FEATURE: Hold Arrows allow you to not hold the button for a second without 
     NG'ing them.
CHANGE: Regular Arrows on the same line as a Hold Arrow now appear green.
NEW FEATURE: Hold Arrows now turn gray player doesn't hold the button.
BUG FIX: #510980 Danger is a static box, not text

------------------------Version 1.57-------------------------
NEW FEATURE: smzip package installer and manager
CHANGE: Installer now uninstalls previous version.
BUG FIX: Package Manager now converts all non-HTTP-friendly characters to '_' when 
     exporting.
CHANGE: Much better texture loading algorithm.  Texture quality and loading time of 
     non-power of 2 texture improved greatly.
CHANGE: New 5th Mix theme graphics.  Thanks thingmabobr!
BUG FIX: #507170 Speed Problem (Got rid of "pauses" caused by buffered writing to 
     log)
BUG FIX: #506309 Installer doesn't overwrite files
BUG FIX: #511187 old banners are shown diagonal

------------------------Version 1.56-------------------------
BUG FIX: 506314 Fixed "Hall of mirrors" on nVidia cards (actually, just a work 
     around for a driver bug)
CHANGE: Couple and Solo now appear in Select Style
CHANGE: New Title Menu options (but you can choose them yet :-)
BUG FIX: Fixed jibberish at the top of Arial Bold font by shifting all letters 
     up one pixel
BUG FIX: 505896 Menu Problems [unreadable/illegible font?]
BUG FIX: Ugly textures with Voodoo3
BUG FIX: Ignore Joystick D-pad now works again.
BUG FIX: Installer-created icon "Open Songs Folder" now fixed for OSes that had 
     problems.
BUG FIX: BMS parse crash fixed.
BUG FIX: Player2 couldn't press up to select different steps
BUG FIX: Player2's side in StepsSelector no longer has multiple bouncing difficulty 
     icons
BUG FIX: 507380 Multiple FAILED with Esc presses
BUG FIX: 507860 - Esc in No Fail Mode = lock up
CHANGE: Improved response time to user input when change sort in SelectMusic
BUG FIX: Announcer no longer makes dancing comments while FAILED is showing.
OPTIMIZATION: Added texture alpha hints - significantly improved texture quality 
     for 16-bit textures.

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
OPTIMIZATION: Preview in SelectMusic now uses banner instead of background for faster 
     loading
NEW FEATURE: Change resolution, display color depth, texture color depth, and toggle 
     texture filtering
NEW FEATURE: Check at startup time that all required theme elements are present
NEW FEATURE: installer
NEW FEATURE: motion blur "Failed" effect
NEW FEATURE: MusicWheel remembers previously selected song.
BUG FIX: Back from Result screen no longer leads to black screen
CHANGE: All documentation is now in "Docs" folder
CHANGE: New "Danger" animation

------------------------Version 1.50-------------------------
Bug fix: No longer crashes on foreign chars in BMS files. Please report all songs that 
     still cause a crash. 
Change: Added sepatate background movie property to Song. Now status backgrounds will 
     display in SelectMusic if the Song has a movie background. 
Change: Asset directory structure has completely changed in preparation for theme support. 
Optimization: BitmapText is now ~80% faster. 
New Feature: 5th Mix style SelectMusic and auto-generated text banners. Loads 100x faster 
     than the previous SelectSong screen. 
Change: Error messages in Release builds no longer show source file line and number. 
Bug Fix: Fade transition now fades all the way to black (doesn't leave a "hazy" screen). 
Bug Fix: Multiple joysticks now work again. 

-------------------------Version 0.97-------------------------
Bug fix: Songs with same title no longer change positions while scrolling.
Bug fix: Wrong group name for DWI files in the Songs folder.
Bug fix: Fixed a problem that caused the music to become out of sync if many sound 
     effect are played.  This was causing the song to become out of sync near the end.  
     Note that this change makes the music much more accurate, and some BMS files may 
     be out of sync if they appeared in sync before.
Bug fix: Dancing waits 2 seconds before starting music playback.  This allows some 
     slower systems to catch up.
Bug fix: Little now works correctly.  It is accessible from the PlayerOptions menu.
Optimization: Lower priority for movie decoding, which is a major speed boost on 
     slower computers.
Change: Freeze arrow scoring now matches DDR MAX.  The start of a freeze arrow is judged 
     as other steps are (perfect, great, good, etc).  Missing the start of a freeze will 
     should break your combo, but NG'ing the freeze arrow will not. 
New feature: Basic song statistics tracking.  Times played, top score, top combo, and 
     top grade are shown on SelectSong screen.
New feature: Assist tick, accessible from the SongOptions menu. 
New feature: Music Rate, accessible from the SongOptions menu.  You can speed up or 
     slow down the rate of music playback.  Great for practicing hard step patterns.
New feature: Freeze arrow scores are now shown on the Results screen.
Change: Arrow ghost now appear different depending on the combo length (roughly matches 
     DDR MAX).
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
Optimization:  In SelectSong, only one preview graphic is drawn.  This saves fill rate 
     and leaves room for group/sort/BPM text.
Change: DWI files now use the DWI directory structure.  StepMania also automatically 
     crop and rotate the song banner.  BMS files still reside in the Songs folder.
Change: Now hold Next to get to PlayerOptions and SongOptions menus  (Previously, you 
     would hold Next to bypass these menus).
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
23. Thanks
************************************************************

Thanks to:
	Warrior Bob (Dave H) for helping with the readme!
	Dj Slash & Tony Thai for the caution graphic
	Kyle "KeeL" Ward for his awesome menu music (www.ampcast.com/keel)
	Gotetsu for helping track down bugs and other various tasks

************************************************************
End of document
************************************************************
