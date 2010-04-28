-- supported aspect ratios; some of which I will ignore
-- only really pay attention to any ratio marked with a star;
-- I don't think anyone uses 3:4, 1:1, or 8:3.
-- (Archer added 5:4 later and I didn't account for it yet.)
AspectRatios = {
	ThreeFour   = 0.75,		-- (576x760 at 1024x768)
	OneOne      = 1.0,		-- 480x480  (uses Y)
	FiveFour    = 1.25,		-- 600x480 (1280x1024 is a real use case)
	FourThree   = 1.33333,	--* 640x480
	SixteenTen  = 1.6,		--* 720x480
	SixteenNine = 1.77778,	--* 853x480
	EightThree  = 2.66666,	-- 1280x480
};

function IsUsingWideScreen()
	return GetScreenAspectRatio() >= 1.6;
end;

-- take and use it as you like, I don't care -aj
-- (although I should mention this file was specific to moonlight and was pretty
-- bad before some editing. -aj)

-- this one is good though:
function WideScale(AR4_3, AR16_9) return scale( SCREEN_WIDTH, 640, 854, AR4_3, AR16_9 ); end