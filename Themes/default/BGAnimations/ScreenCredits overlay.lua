local function Fooled()
	local phrases = {
		"hornswaggled",
		"bamboozled",
		"hoodwinked",
		"swindled",
		"duped",
		"hoaxed",
		"fleeced",
		"shafted",
		"caboodled",
		"beguiled",
		"finagled",
		"two-timed",
		"suckered",
		"flimflammed"
	}
	return phrases[math.random(#phrases)]
end

local ssc = {
	"AJ Kelly as freem",
	"Jonathan Payne (Midiman)",
	"Colby Klein (shakesoda)",
}

local sm_ssc = {
	"Jason Felds (wolfman2000)", -- Timing Segments, Split Timing, optimization
	"Thai Pangsakulyanont (theDtTvB)", -- BMS, Split Timing, optimization
	"Alberto Ramos (Daisuke Master)",
	"Jack Walstrom (FSX)",
}

local stepmania = {
	"Chris Danford",
	"Glenn Maynard",
	"Steve Checkoway",
	-- and various other contributors
}

local oitg = {
	"infamouspat",
	"Mark Cannon (vyhd)",
}

local contrib = {
	"Aldo Fregoso (Aldo_MX)", -- delays
	"cerbo",
	"cesarmades", -- pump/cmd* noteskins
	"Chris Eldridge (kurisu)", -- dance-threepanel
	"Christophe Goulet-LeBlanc (Kommisar)", -- songs
	"corec", -- various fixes
	"galopin", -- piu PlayStation2 usb mat support
	"gholms", -- automake 1.11 support
	"juanelote", -- SongManager:GetSongGroupByIndex, JumpToNext/PrevGroup logic mods
	"Kaox", -- pump/default noteskin
	"NitroX72", -- pump/frame noteskin
	"Petriform", -- Music
	"sy567", -- beginner helper fix
	"v1toko", -- x-mode from StepNXA
	"waiei", -- custom scoring fixes + Hybrid scoring
}

local translators = {
	"John Reactor (Polish)",
}

local thanks = {
	"A Pseudonymous Coder", -- support
	"Bill Shillito (DM Ashura)", -- Music (not yet though)
	"cpubasic13", -- testing (a lot)
	"Dreamwoods",
	"Jason Bolt (LightningXCE)",
	"Jousway", -- Noteskins
	"Matt1360", -- Automake magic + oitg bro
	"Renard",
	"Ryan McKanna (Plaguefox)",
}

local shoutout = {
	"The Lua team", -- lua project lead or some shit. super nerdy but oh hell.
	"Mojang AB", -- minecraft forever -freem
	"Wolfire Games", -- piles of inspiration
	"NAKET Coder",
	"Ciera Boyd", -- you bet your ass I'm putting my girlfriend in the credits
	--Image(), -- we should have some logos probably to look super pro
	"#KBO",
	"Celestia Radio", -- LOVE AND TOLERANCE
	"You showed us... your ultimate dance",
	-- "Can't stop crying... buckets of tears!"
}

local copyright = {
	"StepMania is released under the terms of the MIT license.",
	"If you paid for the program you've been " .. Fooled() .. ".",
	"All content is the sole property of their respectful owners."
}

local sections = {
	{ "the spinal shark collective (project lead)", ssc },
	{ "sm-ssc Team", sm_ssc },
	{ "StepMania Team", stepmania },
	{ "OpenITG Team", oitg },
	{ "Translators", translators },
	{ "Other Contributors", contrib },
	{ "Special Thanks", thanks },
	{ "Shoutouts", shoutout },
	{ "Copyright", copyright },
}

-- To add people or sections modify the above.

local lineOn = cmd(zoom,0.875;strokecolor,color("#444444");shadowcolor,color("#444444");shadowlength,3)
local sectionOn = cmd(diffuse,color("#88DDFF");strokecolor,color("#446688");shadowcolor,color("#446688");shadowlength,3)
local item_padding_start = 4;

local creditScroller = Def.ActorScroller {
	SecondsPerItem = 1.25;
	NumItemsToDraw = 40;
	TransformFunction = function( self, offset, itemIndex, numItems)
		self:y(30*offset)
	end;
	OnCommand = cmd(scrollwithpadding,item_padding_start,15);
}

local function AddLine( text, command )
	local text = Def.ActorFrame{
		LoadFont("Common normal")..{
			Text = text or "";
			OnCommand = command or lineOn;
		}
	}
	table.insert( creditScroller, text )
end

-- Add sections with padding.
for section in ivalues(sections) do
	AddLine( section[1], sectionOn )
	for name in ivalues(section[2]) do
		AddLine( name )
	end
	AddLine()
	AddLine()
end

creditScroller.BeginCommand=function(self)
	SCREENMAN:GetTopScreen():PostScreenMessage( "SM_BeginFadingOut", (t.SecondsPerItem * (#t + item_padding_start) + 5) );
end;


return Def.ActorFrame{
	creditScroller..{
		InitCommand=cmd(CenterX;y,SCREEN_BOTTOM-64),
	},
	LoadActor(THEME:GetPathB("ScreenWithMenuElements","background/_bg top"))..{
		InitCommand=cmd(Center),
	},
};
