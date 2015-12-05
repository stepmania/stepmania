-- This is the example for how to theme ScreenMapControllers, the Config
-- Key/Joy Mappings screen.

-- The contents of this file will actually be in several sections, each
-- intended to be placed in its own file.
-- Sections will be seperated by lines of dashes to make the separation clear.
-- The first line inside the dashes will have the name of the file to copy the
-- section to.
-- The rest of the area inside the dashes will discuss general aspects of the
-- section being discussed.


----------------------------------------------------------------
-- metrics.ini - the metrics you might want to set
-- See the ScreenMapControllers section of _fallback/metrics.ini
----------------------------------------------------------------

----------------------------------------------------------------
-- overlay, underlay, decorations, transitions
-- ScreenMapControllers supports the standard layers and transitions:
-- Layers:  overlay, underlay, decorations
-- Transitions:  in, out, cancel
-- Details of creating the layers and transitions for a screen are covered in
-- ScreenWithLayersAndTransitions.lua.
-- Previous versions of ScreenMapControllers had the warning in the overlay.
-- The warning has been moved to Graphics/ScreenMapControllers warning.lua.  If
-- you have previously made a theme, update your theme to match the new
-- convention.
----------------------------------------------------------------

----------------------------------------------------------------
-- Graphics/ScreenMapControllers warning.lua
-- The warning that is displayed when entering the screen.
-- The _fallback warning uses the WarningHeader and WarningText strings from
-- en.ini.  If all you want to do is change the text, add those entries to your
-- en.ini.
-- The actor returned by your lua file must handle TweenOn and TweenOff commands.
-- TweenOn will occur when the screen starts.
-- TweenOff will occur when the warning is dismissed.
-- Here is a simple actor that dims the screen and displays the text.
----------------------------------------------------------------
return Def.ActorFrame{
	InitCommand=cmd(xy, SCREEN_CENTER_X, SCREEN_CENTER_Y),
	Def.Quad{
		TweenOnCommand=cmd(zoomto, SCREEN_WIDTH, SCREEN_HEIGHT; diffuse, Color.Black),
		TweenOffCommand=cmd(linear, 0.5; diffusealpha, 0),
	},
	Def.BitmapText{
		Font="Common Normal",
		Text=ScreenString("WarningHeader"),
		TweenOnCommand=cmd(y,-80;diffuse,Color.Red),
		TweenOffCommand=cmd(linear,0.5;diffusealpha,0),
		
	},
	Def.BitmapText{
		Font="Common Normal",
		Text=ScreenString("WarningText"),
		TweenOnCommand=cmd(y,10;wrapwidthpixels,SCREEN_WIDTH-48),
		TweenOffCommand=cmd(linear,0.5;diffusealpha,0),
	},
}

----------------------------------------------------------------
-- Graphics/ScreenMapControllers action.lua
-- The fallback actor that is used when there isn't a specific actor for an
-- action.  At a minimum, this should display the name of the action, in order
-- to correctly handle the possibility of actions being added in the future.
-- _fallback's version accomplishes this by being a BitmapText and fetching the
-- text from en.ini.
----------------------------------------------------------------
return Def.BitmapText{
	Font="Common Normal",
	InitCommand= cmd(x, SCREEN_CENTER_X; zoom, .75; diffuse, color("#808080")),
	OnCommand= function(self)
		self:diffusealpha(0)
		self:decelerate(0.5)
		self:diffusealpha(1)
		-- This code is in the OnCommand because the screen sets the name of this
		-- actor after loading it.  If this code was in the InitCommand, it would
		-- not work because the name is blank at that point.
		-- fancy effect:  Look at the name (which is set by the screen) to set text
		-- The name is concatenated with "Action" so that the strings used will be
		-- unique, next to each other in the language file, and clear in usage.
		self:settext(
			THEME:GetString("ScreenMapControllers", "Action" .. self:GetName()))
	end,
	OffCommand=cmd(stoptweening;accelerate,0.3;diffusealpha,0;queuecommand,"Hide"),
	HideCommand=cmd(visible,false),
	GainFocusCommand=cmd(diffuseshift;effectcolor2,color("#808080");effectcolor1,color("#FFFFFF")),
	LoseFocusCommand=cmd(stopeffect),
}

----------------------------------------------------------------
-- Graphics/ScreenMapControllers <action name>.lua
-- Every action can have its own special actor which will be used if it exists.
-- Replace "<action name>" with the lowercase name of the action you are making
-- an actor for and create the actor you want for that action.
-- As of this writing, action names are:
-- "clear", "reload", "save", "setlist", "exit"
-- Creating actors is discussed in Examples/anatomy_of_an_actor.lua, so an
-- example is not provided here.
----------------------------------------------------------------

----------------------------------------------------------------
-- Graphics/ScreenMapControllers nosetlistprompt.lua
-- This is the actor that is displayed when the player attempts to use the
-- SetList (or "Assign List") action without anything in the list.
-- It should tell the player that the list is empty and how to add things to
-- the list.
-- It must handle TweenOn and TweenOff commands.
-- TweenOn occurs when the player tries to perform the action and fails.
-- TweenOff occurs when the prompt is dismissed.
-- _fallback's does this by dimming the screen with a quad and using a
-- BitmapText to display the string from the language file.
-- Note the use of stoptweening in TweenOn and TweenOff.  This is to prevent
-- the player from overflowing the tween stack by mashing.
----------------------------------------------------------------
return Def.ActorFrame{
	InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y),
	Def.Quad{
		InitCommand=cmd(zoomto,SCREEN_WIDTH,SCREEN_HEIGHT;diffuse,Color.Black;diffusealpha,0),
		TweenOnCommand=cmd(stoptweening;diffusealpha,1;linear,0.5;diffusealpha,0.8),
		TweenOffCommand=cmd(stoptweening;linear,0.5;diffusealpha,0),
	},
	Def.ActorFrame{
		Def.BitmapText{
			Font="Common Normal",
			Text=ScreenString("NoSetListPrompt"),
			InitCommand=cmd(y,10;wrapwidthpixels,SCREEN_WIDTH-48;diffusealpha,0),
			TweenOnCommand=cmd(stoptweening;diffusealpha,0;sleep,0.5125;linear,0.125;diffusealpha,1),
			TweenOffCommand=cmd(stoptweening;linear,0.5;diffusealpha,0),
		},
	},
}
