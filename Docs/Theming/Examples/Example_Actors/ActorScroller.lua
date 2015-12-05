-- A simple ActorScroller example with a couple of BitmapText children.
-- ActorScrollers are used to create scrolling displays of other Actors.

Def.ActorScroller{
	-- (float) Number of items to have visible at one time.
	-- ScrollThroughAllItems divides this value by 2.
	NumItemsToDraw=14, -- Default value is 7 if this parameter is omitted.

	-- (float) The number of seconds to show each item.
	SecondsPerItem=1, -- Default value is 1 if this parameter is omitted.

	-- Transforms the ActorScroller's children. Usually used for scrolling.
	-- The most important part of an ActorScroller.
	-- This example spaces each item out 32 pixels vertically.

	-- "offset" represents the offset from the center of the scroller.
	TransformFunction=function(self,offset,itemIndex,numItems)
		self:y(32*offset)
	end,

	-- (int) "1 == one evaluation per position"
	-- default value is 1 if this parameter is omitted.
	--Subdivisions=1,

	-- (bool) A built-in mask for hiding elements. Disabled by default.
	--UseMask=false,

	-- (float) Set the width and height of the mask. Only used if UseMask is true.
	-- Defaults for both are 0.
	--MaskWidth=0,
	--MaskHeight=0,

	-- children (items to scroll)
	-- CAVEAT: BitmapTexts have to be wrapped in ActorFrames in order to be colored.
	-- This is a long standing issue (ITG? ITG2?)
	Def.ActorFrame{
		Def.BitmapText{
			Name="ScrollItem1",
			Font="Common Normal",
			Text="Scroll Item 1",
			InitCommand=cmd(diffuse,color("#FF0000")),
		},
	},
	Def.ActorFrame{
		Def.BitmapText{
			Name="ScrollItem2",
			Font="Common Normal",
			Text="Scroll Item 2",
			InitCommand=cmd(glow,color("#00FF0088")),
		},
	},
	Def.BitmapText{
		Name="ScrollItem3",
		Font="Common Normal",
		Text="Scroll Item 3",
		InitCommand=cmd(bob;effectmagnitude,8,0,4),
	},

	-- Scroller commands
	InitCommand=cmd(Center),
	OnCommand=cmd(scrollwithpadding,8,8),
}
