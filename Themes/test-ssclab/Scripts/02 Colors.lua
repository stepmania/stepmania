-- Colors! --
Color = {
	--[[ Basic Colors ]]
	Red			=	color("#ed1c24"),
	Orange		=	color("#f7941d"),
	Yellow		=	color("#fff200"),
	Green		=	color("#39b54a"),
	Blue		=	color("#00aeef"),
	Purple		=	color("#92278f"),
	--[[ Greyscale Tints ]]
	White		=	color("1,1,1,1"),
	Silver		=	color("0.75,0.75,0.75,1"),
	Grey		=	color("0.5,0.5,0.5,1"),
	Steel		=	color("0.25,0.25,0.25,1"),
	Black		=	color("0,0,0,1"),
	--[[ Transparencies ]]
	Invisible	=	color("1,1,1,0"),
	Stealth		=	color("0,0,0,0"),
	--[[ Alternatives ]]
	HKS = {
	--[[ HKS Z Process ]]
		Red 		= color("#ee3048"),
		Orange	= color("#ffcb05"),
		Yellow	= color("#fff200"),
		Green		= color("#a4cf57"),
		Blue		= color("#00bfe8"),
		Purple	= color("#6e3694"),
		-- There is no "White" for this library;
		White		= color("1,1,1,1"),
		Silver	= color("#b1c6c8"),
		Grey		= color("#748392"),
		Steel		= color("#435769"),
		Black		= color("#262d27"),
		Invisible = color("#b1c6c800"),
	},
	--[[ Theme Elements ]]
	ThemeElement = {
		Background = {
			Primary 		=	color("#b1c6c8"), -- Grey Highlight
			Secondary 		=	color("#748392"), -- Grey Darkened
			Highlight 		=	color("#fff200"), -- Orange Highlight
			Accent	 		=	color("#f99d1c"), -- Orange Darkened
		},
		UI = {
			Primary			=	color("1,1,1,1"),
			Secondary		=	color("#a8aca9"),
			Highlight		=	color("#fff200"),
			Accent			=	color("#ffd400"),
		};
	},
};
Metrics = {}
-- Color Functions --
--[[ function BoostColor(cColor,fBoost,fAlpha)
	local c = cColor;
	local x = fBoost;
	local a = fAlpha;
	c = { c[1]+(c[1]*boost),c[2]+(c[2]*boost),c[3]+(c[3]*boost),c[4] };
	c[4] = (fAlpha ~= nil) and fAlpha or c[4];
	return c;
end; --]]