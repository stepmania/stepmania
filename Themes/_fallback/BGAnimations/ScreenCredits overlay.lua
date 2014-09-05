-- To add a section to the credits, use the following:
-- local theme_credits= {
-- 	name= "Theme Credits", -- the name of your section
-- 	"Me", -- The people you want to list in your section.
-- 	"Myself",
-- 	"My other self",
--  {logo= "pro_dude", name= "Pro self"}, -- Someone who has a logo image.
--     -- This logo image would be "Graphics/CreditsLogo pro_dude.png".
-- }
-- StepManiaCredits.AddSection(theme_credits)
--
-- If you want to add your section after an existing section, use the following:
-- StepManiaCredits.AddSection(theme_credits, 7)
--
-- Or position can be the name of a section to insert after:
-- StepManiaCredits.AddSection(theme_credits, "Special Thanks")
--
-- Or if you want to add your section before a section:
-- StepManiaCredits.AddSection(theme_credits, "Special Thanks", true)

-- StepManiaCredits is defined in _fallback/Scripts/04 CreditsHelpers.lua.

local lineOn = cmd(zoom,0.875;strokecolor,color("#444444");shadowcolor,color("#444444");shadowlength,3)
local sectionOn = cmd(diffuse,color("#88DDFF");strokecolor,color("#446688");shadowcolor,color("#446688");shadowlength,3)
local item_padding_start = 4;
local line_height= 30
-- Tell the credits table the line height so it can use it for logo sizing.
StepManiaCredits.SetLineHeight(line_height)

local creditScroller = Def.ActorScroller {
	SecondsPerItem = 0.5;
	NumItemsToDraw = 40;
	TransformFunction = function( self, offset, itemIndex, numItems)
		self:y(line_height*offset)
	end;
	OnCommand = cmd(scrollwithpadding,item_padding_start,15);
}

-- Add sections with padding.
for section in ivalues(StepManiaCredits.Get()) do
	StepManiaCredits.AddLineToScroller(creditScroller, section.name, sectionOn)
	for name in ivalues(section) do
		StepManiaCredits.AddLineToScroller(creditScroller, name)
	end
	StepManiaCredits.AddLineToScroller(creditScroller)
	StepManiaCredits.AddLineToScroller(creditScroller)
end

creditScroller.BeginCommand=function(self)
	SCREENMAN:GetTopScreen():PostScreenMessage( 'SM_MenuTimer', (creditScroller.SecondsPerItem * (#creditScroller + item_padding_start) + 10) );
end;

return Def.ActorFrame{
	creditScroller..{
		InitCommand=cmd(CenterX;y,SCREEN_BOTTOM-64),
	}
};
