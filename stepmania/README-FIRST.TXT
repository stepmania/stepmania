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
14. Backgrounds
15. BGAnimations format
16. The .CRS File Format
17. Extra Stages
18. Creating an Announcer
19. Creating a Note Skin
20. Creating a Theme
21. How StepMania Loads Textures
22. Building StepMania from CVS Source
23. Thanks

See the file "NEWS" for version history.


************************************************************
1. About StepMania
************************************************************

StepMania is capable of playing many game types.  Currently, it supports games 
similar to Dance Dance Revolution, Pump It Up, EZ 2 Dancer, and Para Para 
Paradise.  In the future, it will support games similar to BeatMania, Guitar 
Freaks, DrumMania, and more.

The games played by StepMania are rhythm games.  Notes scroll up from the 
bottom of the screen, and the player must hit the corresponding button on the 
controller in time to the music.  All games can be played using the keyboard, 
but the real fun comes when using specially designed controllers, like a dance 
pad or hand sensors.


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
* DivX 5 video codec for movie visualizations (www.divx.com)


************************************************************
3. DirectX
************************************************************

DirectX 8.1 is required before installing StepMania. If DirectX 8.1 is not 
installed on your computer, the StepMania installer will display a warning 
and abort the installation.


************************************************************
4. Installation
************************************************************

To install StepMania, download the file stepmania300xxxx.exe to a known 
location on your hard drive, then double-click the file icon to begin the 
installation.The installer automatically remove an older version of the 
program if one is present. The default installation directory is 
'C:\Program Files\StepMania'.


************************************************************
5. Controls
************************************************************

Use the "Config Key/Joy" option from the Title Menu to configure your keyboard,
dance pad, or other game controller.  This menu allows you to map up to 2 
keyboard/joystick buttons to each function in the game.  The third column is 
the default key mapping, and cannot be changed.  However, you can override a 
default keyboard key by simply assigning it to another function.  The mappings 
for player 1 are on the left half of the screen, and the mapping for player 2 
are on right half.


The following is a list of special keys:

Any time:
  * F4 = toggle fullscreen
  * F5 = toggle detail
  * Hold Tab = increase game speed 4x (useful for moving through menus quickly)
  * Hold Tilde (~) = decrease game speed to 1/4x

In menus:
  * Arrow keys navigate menus
  * Enter = Start
  * Escape = Back

In gameplay:
  * F8  = toggle AutoPlay
  * F9  = decrease offset
  * F10 = increase offset
  * F11 = decrease BPM of current segment
  * F12 = increase BPM of current segment
  * Hold Escape = abort playing


************************************************************
6. How StepMania Loads Songs
************************************************************

All files for a specific song (data file, music file, banner, background) 
must reside in a single folder.  This folder must have the following path:

   Songs/<group folder>/<song folder>/

For example, the song B4U may consist of the following files:

   Songs/Dance Dance Revolution 4th Mix/B4U/b4u.dwi
   Songs/Dance Dance Revolution 4th Mix/B4U/b4u.mp3
   Songs/Dance Dance Revolution 4th Mix/B4U/b4u-banner.png
   Songs/Dance Dance Revolution 4th Mix/B4U/b4u.png
   Songs/Dance Dance Revolution 4th Mix/B4U/b4u-movie.avi

StepMania supports the SM, DWI, BMS, and KSF song file formats.  If the song
file format is BMS or KSF, simply put all the KSF or BMS files into the song 
folder.

If a song file does not specify a valid banner file, StepMania first looks 
for a graphic in the song folder that contains the word "banner".  If that
search fails, StepMania will guess which image is most appropriate.

If a song file does not specify a valid background file, StepMania first looks 
for a graphic in the song folder that contains the words "bg" or "background".
If that search fails, StepMania will guess which image is most appropriate.

If a song file does not specify a valid CDTitle file, StepMania will look
for a graphic in the song folder that contains the words "cdtitle".  If that 
search fails, StepMania will guess which image is most appropriate.

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

* Highly Recommended:  specific brands of PlayStation->USB adaptor

There is just one brand of PlayStation->USB adaptor that functions perfectly 
with StepMania.  These adaptors are available for purchase through some 
importers (www.liksang.com).  The adaptor is made by "Boom", and it small and 
black with a detachable USB cable.  These adaptors cost about $15 and allow 
you to attach one dance pad per adaptor.  The primary advantage of these 
adaptors is that they do not require special drivers and require no 
configuring.

* Semi-Recommended:  PlayStation->Parallel adaptor

PlayStation->Parallel converters do not have the simultaneous button problem 
that USB converters do.  The disadvantage of these adaptors is that home-made 
drivers are required to use these devices.  These drivers are often finicky 
and incompatible with some sound cards.  PlayStation->Parallel adaptors can 
be purchased from the StepMania web site or any importer.

For Windows 98 and Windows Me users, the recommended driver is "DirectPad Pro
5.0 for Win9x".  For Windows 2000 and Windows XP, the recommended drivers is
NTPad XP 1.x or PSXPAD.

Win98/WinME
  "DirectPad Pro 5.0 for Win9x", found at
   http://www.aldostools.com/dpad.html

Win2K/WinXP
NTPad XP 1.x, found at
   http://www.aldostools.com/dpad.html
PSXPAD 0.8.2002.0606b, found at
   http://www.psxpad.com/download/downcnt.php?name=psxp_a020606b


************************************************************
8. Compatibility Issues
************************************************************

Some video card/driver combinations have visual errors when running StepMania. 
If you experience any visual errors, please visit your video card 
manufacturer's web site and download the latest Direct3D 8.1 drivers. Below 
is a list of known issues that are thought to be video card driver bugs:

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
3. Tweak the Display Resolution and Texture Resolution setting in the 
   GameOptions screen.

* Chris, will you tell me when the next version will be released?

I'd rather not going to say.  Sorry.  It's gotten me in trouble before.

* Why won't you tell me?  I promise I won't tell anyone else.

The reasons I won't tell is because: a) I don't know yet, and b) I don't want 
to make anyone angry if it takes longer than expected (it usually does).


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
file in any compression application (e.g. WinZip, WinRAR).

The StepMania Package Manager (smpackage.exe) can also export song packages 
for you.  Simply launch the Package Manager 
(Start Menu->Programs->StepMania->Package Manager), click the items you would 
like to make a package for, then click the Export button.  A .smzip package 
will be placed on your desktop, and you can share it with friends.


************************************************************
11. Questions, Bugs, Suggestions, and Help
************************************************************

If you have a question about or problem with StepMania, please first ask your 
question a StepMania message board.  A link is available from the StepMania 
web site - http://www.stepmania.com.

If you have found a bug, please file a bug report into the SourceForge bug 
tracking system (http://sourceforge.net/tracker/?group_id=37892&atid=421366).  
This will allow the developers to track and fix bugs much more efficiently.  

* In a bug report, please specify your video card brand and model, and your OS 
version (Win98, WinXP, etc).
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
note patterns.  When you choose 'Save', your song will be saved in both the .SM
format and the DWI format for use in Dance With Intensity.  If you save 
a song that was originally in the BMS or KSF format, the old files will be
appended with ".old", and a new .SM file will be written with your new song 
data.

Following are a few basic commands that you will find useful in learning to use
the editor:

Esc - show/dimiss menu
Up/Down - change beat
Left/Right - change beat snap value
Enter - Set selection begin marker
Space - Set selection end marker
P - Play back current selection (if no selection, play whole song)
R - Record over current selection (if no selection, record whole song)


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
    on the Select Music screen.  e.g. "~Dirty Mix~" or "(remix)".
#ARTIST:...;   - The artist of the song.
#TITLETRANSLIT:...;  - Transliteration of song's main title for sorting
#SUBTITLETRANSLIT:...; - Transliteration of song's sub title for sorting
#ARTISTTRANSLIT:...;  - Transliteration of the artist of the song
#CREDIT:...;   - Give yourself some credit here for creating a wonderful song.
#BPM:...;      - BPM of the music
#BANNER:...;   - The file name of the banner image.  e.g. "b4u-banner.png".  
    This image must reside in the song folder.
#BACKGROUND:...;   - The file name of the background image.  e.g. "b4u-bg.png". 
    This image must reside in the song folder.
#CDTITLE:...;   - The file name of the spinning CD logo.  e.g. "b4u-cdtitle.png".  
    This image must reside in the song folder.
#MOVIEBACKGROUND:...;   - The file name of the a background movie.  e.g. 
    "b4u-movie.avi".  This movie must reside in the song folder.
#MUSIC:...;   - The file name of the music file.  e.g. "b4u.mp3".  This image 
    must reside in the song folder.
#MUSICBYTES:...;   - The size in bytes of the music file that this song was last 
    synchronized with.  A "Song may be out of synch" warning will be displayed if 
    the value of this tag does not match the actual size of the music file.
#OFFSET:...;   - The time in seconds at which beat 0 occurs in the music.  This 
    is specified as a floating point value.  e.g. "2.34".
#SAMPLESTART:...;   - The time in seconds to start the music sample that plays 
    on the Select Music screen.  This is specified as a floating point value.  
	e.g. "32.34".
#SAMPLELENGTH:...;   - The time in seconds let the sample music play after 
    starting.  This is specified as a floating point value.  e.g. "32.34".  Note 
    that in the last 1 second of playing the music will fade out.
#SELECTABLE:...;   - If "NO", the song can not be selected manually and can
    only be played as part of a course.  If "ROULETTE", the song can can also be
    selected via roulette.  The default value is "YES".
#BPMS:...; - A value of the format "beat=bpm".  Indicates that at 'beat', the 
    speed of the arrows will change to "bpm".  Both of these values are specified 
    as floating point values.  You must specifiy a BPM value for beat 0.  Multiple 
    BPMs can be given by separating them with commas.  e.g. "0=160,120=80".  
#STOPS:...;   - A value of the format "beat=sec".  Indicates that at 'beat', 
    the motion of the arrows should stop for "sec" seconds.  Both of these values 
    are specified as floating point values.  Multiple stops can be given by 
    separating them with commas.  e.g. "60=2.23,80=1.12".
#BGCHANGE:...;   - A value of the format "beat=bg name".  Indicates that at 'beat', 
	the background should begin playing 'bg name'.  'beat' is a fractional value
	value and 'anim name' is a string.  Different animation change are separated 
	by commas.  e.g. "60=falling,80=flower".


Each pattern of notes has have the same basic format:

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

NoteData:  This value requires a longer explanation.

Each note is represented by a character:

0 = no note here
1 = a regular "tap note"
2 = beginning of a "hold note"
3 = end of a "hold note"
a-z,A-z = tap notes reserved for game types that have sounds associated with notes

Notes that are hit at the same time are grouped into rows.  For example, if the  
NotesType is "dance-single", the row "1001" would specify that both the Left 
and Right and Down panels should be hit at the same time.

The number of notes per row (also called the number of 'columns') depends on 
the "NotesType.

dance-single = 4 notes/row  (Left,Down,Up,Right)
dance-double = 8 notes/row
dance-couple = 8 notes/row
dance-solo = 6 notes/row
pump-single = 5 notes/row
pump-double = 10 notes/row

Note rows are grouped into measures.  The number of note rows you specify in a 
measure will determine the time value of each note.  For example, if there are 
4 note rows in a measure, each note will be treated as a quarter note.  If there 
are 8 notes rows in a measure, each note will be treated as a eighth note.  If 
there are 12 notes rows in a measure, each note will be treated as a triplet 
(1/12th) note.  Measures are separated by a comma.

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
14. Backgrounds
************************************************************

StepMania supports three different modes of backgrounds:

BGAnimations - The background will cycle randomly through different sprite 
animations.  Each of these animation must reside in the "BGAnimations" 
folder in the StepMania program directory.  See the section called 
"BGAnimations" in the readme for more information on the BGAnimations format.
BGAnimations will perform better than the other background modes on slower 
computers.

Random Movies - The background will cycle randomly over any avi or mpg files 
in the "RandomMovies" folder in the StepMania program directory.

Visualizations - A random avi or mpg file will be chosen from the 
"Visualizations" folder in the StepMania program directory.  This movie file 
will be played and blended over top of the song's background graphic.

Many video AVIs will require the DivX codec for playback.  If you haven't yet
downloaded DivX, you can get it from www.divx.com.  Playing movies as the 
background is very processor intensive, and may cause the frame rate to 
decrease on slower computers.

Sample animations, visualizations, and random movies can be found at 
www.stepmania.com.


************************************************************
15. BGAnimations Format
************************************************************

The BGAnimations folder in the StepMania program directory may contain several
BGAnimation folders.

\BGAnimations\<anim folder>\<layer files>

<anim folder> is the name of the animation.

<layer files> are one or more graphics files that will be used as layers in the 
animation.  Each sprite represents exactly one layer.  Multiple layers can be 
specified by having multiple <layer files>.  

For example, a BGAnimation named "flower" may include the files:

	BGAnimations\flower\1 TileScrollUp.png
	BGAnimations\flower\2 StartOnRandomFrame ParticlesFloatDown 1x2.png
	BGAnimations\flower\3 TileScrollLeft CycleColor 2x2.png

The first number in the name of a <layer file> will specifies its drawing 
order.  A layers with a small number will be drawn before a layer with a larger
number.

Different "effects" are specified for each layer by adding tokens to the file 
name.  Here are an explanation of the currently supported tokens:

UseSongBg - Use the song's background file instead of this graphics file.  
Since this graphics file is merely a placeholder, you might want to make this 
graphic a 1x1 graphic to save disk space.

Add - use additive blending instead of normal blending.

CycleColor - cycle the color of the layer over the colors of the rainbow

CycleAlpha - cycle the alpha channel of the layer over type

StartOnRandomFrame - Start the animation on a random frame.  This is mainly 
useful for Tile or Particle effects.  

DontAnimate - Stay on the first frame of this animation.  This is only useful 
for layers with multiple frames.

(stretch effects) - These scretch the graphic across the extire screen.
ScretchScrollLeft,
StretchScrollRight,
StretchScrollUp,
StretchScrollDown,
       - Scroll the layer in a direction

StretchWater,
StretchBubble,
StretchTwist,
StretchSpin,
       - Deform the background sprite with an effect.  Only stretch spin is currently implemented.

(particle effects) - Use the graphic to make individual particles on the screen.
ParticlesSpiralOut,
ParticlesSpiralIn,
       - Spiral particles away from/toward the center of the screen.
ParticlesFloatUp,
ParticlesFloatDown,
ParticlesFloatLeft,
ParticlesFloatRight,
       - Scroll the particles across the screen.
ParticlesBounce,
       - Particles start traveling in random directions and bounce when they hit the edge of the screen.

(tile effects) - Tile the graphics across the screen, forming a grid of graphics.
TileStill,
TileScrollLeft,
TileScrollRight,
TileScrollUp,
TileScrollDown,
       - Scroll the tiles

TileFlipX,
TileFlipY,
       - Flip the tiles along the X or Y axis.

TilePulse,
       - Tiles zoom in and out.


************************************************************
15. The .CRS File Format
************************************************************

CRS files define the courses used in the "Oni" Nonstop Challenge mode.  All CRS
files must reside in the "Courses" folder in the StepMania program folder.

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

You can manually specify extra stages for each song group by creating .CRS
files called "extra1.crs" and "extra2.crs".  These courses must contain
exactly one song in the course.  Don't forget to specify modifiers - see the 
CRS File Format section for more information.  If a CRS file is not found or
is invalid, the program will choose an appropriate song, notes, and modifiers.

Don't forget: extra stage CRS files belong in the song group folder 
(e.g. "Songs/DDRMAX/"), not in "Courses".


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
will appear less blurry than 285x119).  The reason behind this has to do with
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

The DirectX 8.1 SDK is a free (but large) download from Microsoft: 
http://msdn.microsoft.com/downloads/?url=/downloads/sample.asp?url=/MSDN-FILES/
027/001/771/msdncompositedoc.xml&frame=true.  Install it using all the default 
settings.  The SDK installer will configure Visual C++ with the correct paths 
needed to find the DirectX 8.1 libraries and headers.

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
22. Thanks
************************************************************

Thanks to:
	redcrusher for help on ez2 graphics
	Dj Slash & Tony Thai for the caution graphic
	Kyle "KeeL" Ward for his awesome menu music (www.ampcast.com/keel)

************************************************************
End of document
************************************************************
