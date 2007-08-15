-- I'm not comfortable including assets or code from people who want to keep their
-- name a secret. If they believe their contribution places them at any kind of
-- risk, then we should be just as wary. Also, it often makes it hard to track
-- people down a year or more later if necessary.

local graphics = {
	"Lucas “v1ral” Tang",
	"SPiGuMuS",
	"Visage",
	"Ryan “Plaguefox” McKanna",
	"Lamden “Cloud34” Travis",
	"Michael “Redcrusher” Curry",
	"Steve “healing_vision” Klise",
	"Mauro Panichella",
	"Popnko",
	"Griever (Julian)",
	"Miguel Moore",
	"Dj “AeON ExOdus” Washington",
	"Xelf",
	"James “SPDS” Sanders",
	"k0ldx",
}

local theme = {
	"AJ “AJ 187” Kelly",
	"Dan “dieKatze88” Colardeau",
	"Mike “mDaWg” Calfin",
}

local web = {
	"Brian “Bork” Bugh",
}

local sound = {
	"Kyle “KeeL” Ward",
	"Jim “Izlude” Cabeen",
	"Sanxion7",
}

local programming = {
	"Chris Danford",
	"Frieza",
	"Glenn Maynard",
	"Bruno Figueiredo",
	"Peter “Dro Kulix” May",
	"Jared “nmspaz” Roberts",
	"Brendan “binarys” Walker",
	"Lance “Neovanglist” Gilbert",
	"Michel Donais",
	"Ben “Mantis” Nordstrom",
	"Chris “Parasyte” Gomez",
	"Michael “dirkthedaring” Patterson",
	"Sauleil “angedelamort” Lamarre",
	"Edwin Evans",
	"Brian “Bork” Bugh",
	"Joel “Elvis314” Maher",
	"Garth “Kefabi” Smith",
	"Pkillah",
	"Robert Kemmetmueller",
	"Ben “Shabach” Andersen",
	"Will “SlinkyWizard” Valladao",
	"TheChip",
	"David “WarriorBob” H",
	"Mike Waltson",
	"Kevin “Miryokuteki” Slaughter",
	"Thad “Coderjoe” Ward",
	"Steve Checkoway",
	"Sean Burke",
	"XPort",
	"Charles Lohr",
	"Josh “Axlecrusher” Allen",
	"Jason “Wolfman2000” Felds",
	"Eric “Subwire” Gustafson",

}

local thanks = {
	"SimWolf",
	"DJ DraftHorse",
	"Dance With Intensity",
	"BeMaNiRuler",
	"DDRLlama",
	"DDRManiaX",
	"NMR",
	"DJ Paranoia",
	"DJ Yuz",
	"Reo",
	"Random Hajile",
	"Chocobo Ghost",
	"Tyma",
	"Deluxe",
	"Lagged",
	"The Melting Pot",
	"DDRJamz Global BBS",
	"Eric “WaffleKing” Webster",
	"Gotetsu",
	"Mark “Foobly” Verrey",
	"Mandarin Blue",
	"Kenny “AngelTK” Lai",
	"curewater",
	"Bill “DMAshura” Shillito",
	"Illusionz - Issaquah, WA",
	"Quarters - Kirkland, WA",
	"Pier Amusements - Bournemouth, UK",
	"Westcliff Amusements - Bournemouth, UK",
}

local sections = {
	{ "☆☆GRAPHICS☆☆", graphics },
	{ "☆☆THEME CODE☆☆", theme },
	{ "☆☆WEB DESIGN☆☆", web },
	{ "☆☆SOUND☆☆", sound },
	{ "☆☆PROGRAMMING☆☆", programming },
	{ "☆☆SPECIAL THANKS TO☆☆", thanks },
}

-- To add people or sections modify the above.

local fontPath = THEME:GetPathF( "ScreenCredits", "titles" )
local lineOn = cmd(zoom,0.5)
local sectionOn = cmd(zoom,0.5;diffuse,1,.5,.5,1)
local otherOn = cmd(zoom,0.5;diffuse,.5,.5,1,1)
local t = Def.ActorScroller {
	SecondsPerItem = 0.40;
	NumItemsToDraw = 40;
	TransformFunction = function( self, offset, itemIndex, numItems)
		self:y(30*offset)
	end;
	OnCommand = cmd(scrollwithpadding,12,0);
}

local function AddLine( text, command )
	local text = Def.BitmapText {
		_Level = 2;
		File = fontPath;
		Text = text or "";
		OnCommand = command or lineOn;
	}
	-- XXX: Hack. Wrap in an ActorFrame so OnCommand works
	table.insert( t, Def.ActorFrame { text } )
end

-- Add header and padding
AddLine( "STEPMANIA TEAM", otherOn )
AddLine()
AddLine()
AddLine()

-- Add sections with padding.
for section in ivalues(sections) do
	Trace( "steve: " .. section[1] )
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

AddLine( "Join the StepMania team", otherOn )
AddLine( "and help us out!" )

return t;
