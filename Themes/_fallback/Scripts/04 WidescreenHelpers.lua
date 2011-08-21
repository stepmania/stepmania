-- supported aspect ratios
AspectRatios = {
	ThreeFour   = 0.75,		-- (576x760 at 1024x768; meant for rotated monitors?)
	OneOne      = 1.0,		-- 480x480  (uses Y value of specified resolution)
	FiveFour    = 1.25,		-- 600x480 (1280x1024 is a real use case)
	FourThree   = 1.33333,	-- 640x480 (common)
	SixteenTen  = 1.6,		-- 720x480 (common)
	SixteenNine = 1.77778,	-- 853x480 (common)
	EightThree  = 2.66666	-- 1280x480 (two monitors)
}

function IsUsingWideScreen()
	return GetScreenAspectRatio() >= AspectRatios.SixteenTen
end

function IsUsingTwoScreens()
	return GetScreenAspectRatio() == AspectRatios.EightThree
end

-- take and use it as you like, I don't care -aj
-- (although I should mention this file was specific to moonlight and was pretty
-- bad before some editing. -aj)

-- this one is good though:
function WideScale(AR4_3, AR16_9)
	return scale( SCREEN_WIDTH, 640, 854, AR4_3, AR16_9 )
end