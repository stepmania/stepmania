-- General layout:  There is one choice for each possible background fitting
-- mode.  Each choice will have three examples showing how backgrounds of
-- common aspect ratios will be stretched or clipped by that fitting mode.

-- fit_choices will be used later to tell the actor that controls input what
-- actors to use for each choice.
local fit_choices= {}
-- actors will contain all the actors on the screen.
local actors= {}
-- width_per_choice is how much of the screen width will be used to separate
-- each choice
local width_per_choice= _screen.w / #BackgroundFitMode
-- xstart is the x position of the first choice.  It starts at
-- width_per_choice / 2 because that's the x coordinate of the center of the
-- first choice.
local xstart= width_per_choice / 2
-- mini_screen_w and mini_screen_h are the size of the miniature screen used
-- in the examples.
local mini_screen_w= _screen.w * .1
local mini_screen_h= _screen.h * .1

-- This function returns a BitmapText that will be used to label each example
-- in each choice.
-- w and h are the width and height of the aspect ratio, passed in so they
-- can be displayed to be read.
function BGFitNormalExampleText(w, h)
	return Def.BitmapText{
		-- To use a different font for the text, change the Font field.
		Name= "example_label", Font= "Common Normal",
		-- The "BG" part of the text is fetched with THEME:GetString so that it
		-- can be translated.
		Text= w .. ":" .. h .. " " .. THEME:GetString("ScreenSetBGFit", "BG"),
		InitCommand= function(self)
			-- This positions the text underneath its corresponding example.
			self:y(mini_screen_h * .5 + 9)
			self:zoom(.375)
		end,
		OnCommand=cmd(shadowlength,1)
	}
end

-- This loop goes through all the array elements of the BackgroundFitMode
-- table and creates a choice actor for each one.  The BackgroundFitMode
-- table contains an entry for each background fitting mode.
for i, mode in ipairs(BackgroundFitMode) do
	actors[#actors+1]= Def.ActorFrame{
		Name= mode, InitCommand= function(self)
			-- This adds the actor to fit_choices so it can be used by the input
			-- controller.
			fit_choices[i]= self
			-- This positions the choice on the screen.  Choices are placed in a
			-- row across the center of the screen, evenly spaced.
			self:xy(xstart + ((i-1) * width_per_choice), _screen.cy)
		end,
		Def.BitmapText{
			-- This actor is a label for the choice, so the player knows the name
			-- of their choice.
			Name= "mode_label", Font= "Common Normal",
			Text= THEME:GetString("ScreenSetBGFit", ToEnumShortString(mode)),
			InitCommand= function(self)
				-- Position the label above the topmost example.
				self:y(mini_screen_h * -2.5)
				self:zoom(.75)
			end,
			OnCommand=cmd(diffusebottomedge,color("0.875,0.875,0.875");shadowlength,1)
		},
		-- BGFitChoiceExample is a function that creates an example to show how
		-- a bg with a given aspect ratio is affected by the fitting mode for
		-- this choice.  It takes a number of parameters that control how the
		-- example is created.
		-- The simplest way to modify it to suit your theme is to just change
		-- the png files and the two colors.
		BGFitChoiceExample{
			-- exname is the name of the image in Graphics that will be used as an
			-- example bg to show distortion/cropping.  "16_12_example.png" means
			-- that "Graphics/ScreenSetBGFit 16_12_example.png" will be loaded.
			exname= "16_12_example.png",
			-- x and y are the position this example will be placed at.
			-- x is 0 because the x positioning was handled above, when the choice
			-- this example is part of was positioned.
			-- This is the first example of 3, so y is set to position it above
			-- the other two.
			x= 0, y= mini_screen_h * -1.5,
			-- mini_screen_w and mini_screen_h are the size of the miniature screen
			-- the example will draw.
			mini_screen_w= mini_screen_w, mini_screen_h= mini_screen_h,
			-- example_function is the background fitting function for the mode
			-- that will be set by this choice.  bg_fit_functions is a table
			-- that contains a function for each mode, so we set example_function
			-- to the element of bg_fit_functions that has the same name as the
			-- mode this choice is for.
			example_function= bg_fit_functions[mode],
			-- example_label is an actor that will be used to label this example,
			-- so the player knows what size of bg is shown in this example.
			example_label= BGFitNormalExampleText(16, 12),
			-- sbg_color is the color of the quad representing the screen in the
			-- example.  sbg_color is the color of the outline representing the
			-- screen in the example.
			-- The quad is visible behind the example bg, so it's seen around the
			-- edges when the example doesn't fill the miniature screen.
			-- The outline is drawn over everything else so the player can see what
			-- part of the example bg will be cut off.
			sbg_color= color("0,0,0,1"), soutline_color= color("#39b54a")},
		-- The parameters passed to the other examples have the same meaning as
		-- above.
		BGFitChoiceExample{
			exname= "16_10_example.png", x= 0, y= 0,
			mini_screen_w= mini_screen_w, mini_screen_h= mini_screen_h,
			example_function= bg_fit_functions[mode],
			example_label= BGFitNormalExampleText(16, 10),
			sbg_color= color("0,0,0,1"), soutline_color= color("#39b54a")},
		BGFitChoiceExample{
			exname= "16_9_example.png", x= 0, y= mini_screen_h * 1.5,
			mini_screen_w= mini_screen_w, mini_screen_h= mini_screen_h,
			example_function= bg_fit_functions[mode],
			example_label= BGFitNormalExampleText(16, 9),
			sbg_color= color("0,0,0,1"), soutline_color= color("#39b54a")},
	}
end

-- LoseFocus and GainFocus will be used on each choice to show which one
-- is currently selected.
local function LoseFocus(self)
	-- We use stoptweening so that the player can't overflow the tween buffer
	-- through repeated rapid input.  Using stoptweening means that the actor
	-- will stop at its current state and start tweening towards the new state
	-- we are about to add to the tween stack.
	-- If finishtweening was used, there would be jerking from the actor
	-- suddenly reaching the end tween state.
	self:stoptweening()
	-- .25 seconds is a fine time for changing zoom size.
	self:smooth(.125)
	-- Unfocused choices are normal size.
	self:zoom(1)
end

local function GainFocus(self)
	self:stoptweening()
	self:smooth(.125)
	-- The choice with focus is 1.5 size.
	self:zoom(1.25)
end

-- BGFitInputActor returns the actor that will handle input from the player
-- and apply their choice to the preference.  We pass it fit_choices so that
-- it has the choice actors so it can show the player which once they have
-- selected.
actors[#actors+1]= BGFitInputActor(fit_choices, LoseFocus, GainFocus)
return Def.ActorFrame(actors)
