-- I'm not comfortable including assets or code from people who want to keep their
-- name a secret. If they believe their contribution places them at any kind of
-- risk, then we should be just as wary. Also, it often makes it hard to track
-- people down a year or more later if necessary.

local graphics = {
	"Chris Danford",
	"Ryan “Plaguefox” McKanna",
}

local theme = {
	"Chris Danford",
}

local web = {
	"Chris Danford",
}

local sound = {
	"Sanxion7",
	"Kyle “KaW” Ward",
}

local programming = {
	"Glenn Maynard",
	"Chris Danford",
	"Steve Checkoway",
	"(remake this list)",
}

local smo = {
	"Charles Lohr",
	"Josh Allen",
}

local thanks = {
	"The players and fans!",
}

local sections = {
	{ "☆☆GRAPHICS☆☆", graphics },
	{ "☆☆THEME CODE☆☆", theme },
	{ "☆☆WEB DESIGN☆☆", web },
	{ "☆☆SOUND☆☆", sound },
	{ "☆☆PROGRAMMING☆☆", programming },
	{ "☆☆STEPMANIA ONLINE☆☆", smo },
	{ "☆☆SPECIAL THANKS TO☆☆", thanks },
}

-- To add people or sections modify the above.

local fontPath = THEME:GetPathF( "", "_venacti Bold 15px" )
local lineOn = cmd(strokecolor,color("#624e73BB");shadowcolor,color("#000000FF");shadowlengthx,0;shadowlengthy,3;)
local sectionOn = cmd(diffuse,1,.5,.5,1;strokecolor,color("#624e73BB");shadowcolor,color("#000000FF");shadowlengthx,0;shadowlengthy,3;)
local otherOn = cmd(diffuse,.5,.5,1,1;strokecolor,color("#624e73BB");shadowcolor,color("#000000FF");shadowlengthx,0;shadowlengthy,3;)
local item_padding_start = 12;

local t = Def.ActorScroller {
	SecondsPerItem = 0.4;
	NumItemsToDraw = 40;
	TransformFunction = function( self, offset, itemIndex, numItems)
		self:y(30*offset)
	end;
	OnCommand = cmd(scrollwithpadding,item_padding_start,-0.5);
}

local function AddLine( text, command )
	if text then text=string.upper(text) end
	local text = Def.BitmapText {
		_Level = 2;
		File = fontPath;
		Text = text or "";
		OnCommand = command or lineOn;
	}
	-- XXX: Hack. Wrap in an ActorFrame so OnCommand works
	table.insert( t, Def.ActorFrame { text } )
end

-- Add sections with padding.
for section in ivalues(sections) do
	AddLine( section[1], sectionOn )
	for name in ivalues(section[2]) do
		AddLine( name )
	end
	AddLine()
	AddLine()
	AddLine()
end

-- Add more padding and then the join the team.
for i=1,13 do AddLine() end

AddLine( "Join the StepMania team")
AddLine( "and help us out!" )
AddLine( "www.stepmania.com", otherOn  )

t.BeginCommand=function(self)
	SCREENMAN:GetTopScreen():PostScreenMessage( "SM_BeginFadingOut", (t.SecondsPerItem * (#t + item_padding_start) + 5) );
end;


return t;
