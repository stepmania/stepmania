--[[ supported aspect ratios; some of which I will ignore ]]

-- only really pay attention to any ratio marked with a star;
-- I don't think anyone uses 3:4, 1:1, or 8:3.
-- Archer added 5:4 later and I didn't account for it yet.
AspectRatios = {
	ThreeFour   = 0.75,		-- (576x760 at 1024x768)
	OneOne      = 1.0,		-- 480x480  (uses Y)
	FiveFour    = 1.25,		-- 600x480 (1280x1024 is a real use)
	FourThree   = 1.33333,	--* 640x480
	SixteenTen  = 1.6,		--* 720x480
	SixteenNine = 1.77778,	--* 853x480
	EightThree  = 2.66666,	-- 1280x480
};

--[[ shared internal functions ]]
function round(num, idp)
	if idp and idp > 0 then
		local mult = 10 ^ idp;
		return math.floor(num * mult + 0.5) / mult;
	end;
	return math.floor(num + 0.5);
end

-- xxx: this function only cares about 16:9 and 16:10 so far
-- (8:3 would also count but it would be significantly different)
-- so perhaps maybe have this return a widescreen type
DisplayType = {
	'DisplayType_Normal',
	'DisplayType_WideScreen_16_9',
	'DisplayType_WideScreen_16_10',
	'DisplayType_WideScreen_8_3',
	'DisplayType_Invalid',
};

function IsUsingWideScreen()
	local curAspect = round(GetScreenAspectRatio(),5);
	for k,v in pairs(AspectRatios) do
		if AspectRatios[k] == curAspect then
			if k == "SixteenNine" or k == "SixteenTen" then
				return true;
			else return false;
			end;
		end;
	end;
end;

function IsHighDefinition()
	local dWidth = DISPLAY:GetDisplayWidth();
	local dHeight = DISPLAY:GetDisplayHeight();

	-- lol fixme: 720p is considered the minimum for HD
	if dHeight > 480 then return true;
	else return false;
	end;
end;

--[[ real code ]]

function Actor:PositionX(normalX,widescreenX)
	if IsUsingWideScreen() then
		self:x(widescreenX);
	else
		self:x(normalX);
	end;
end;
-- take and use it as you like, I don't care -aj

function WideScale(AR4_3, AR16_9) return scale( SCREEN_WIDTH, 640, 854, AR4_3, AR16_9 ); end