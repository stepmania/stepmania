-- steps.lua: rough rough draft. Please don't use until finalized.
-- ver 20100116

-- shakesoda is going to be making this more generic for use in rhythm.

local MetaTags = {
	Title = {"Dyamite Rave","(transliteration)"},
	Subtitle = {"Down Bird Sota Mix","(transliteration)"},
	Artist = {"NAOKI","(transliteration)"},
	Genre = {"Scouse House","(transliteration)"},
	License = "CC-BY-NC",
	URL = "http://shakesoda.org/"
}

--[[
	If an int is used in a field that expects a time of some sort, it is
	assumed to use beats. If it's a float, it assumes seconds instead.
]]

local CacheData {
	HasAttacks = true,
	HasBanner = true,
	HasBackground = true,
	HasKeysounds = true,
	HasLyrics = true,
	HasMusic = true,
	SongFileNames = "path/from/song/folder",
	LongestChartLength = 220.000 -- use this to thwart the ogg length patch
}

local MetaData = {
	Attacks = {
		-- start = length, startup time, mod string
		0 = { 10, 1.0, "mod,50% reverse" },
		10 = { 5, 1.0, "mod,70% boost" }
	},
	BannerPath = { "banner.png" }, 		-- can be an image, video, or lua
	Background = {							-- background changes.
		Path = "background.png",	
		Animation = {						-- can have as many changes as needed
			--[[ 
				startTime = { path, rate, offset, commands }
				Any ommitted arguments will default to nil, except for rate, which would use 1.0
			]]
			0 = {"path/to/bga", 1.0, 0.000, 0, cmd(rainbow) },
			0 = {"path/to/bga2", 1.0, 0.000, 0, cmd(thump) }
		},
	},
	--[[
	-- #BGCHANGES2:32.000=flash=1.000=0=0=1=====,
	BackgroundChanges2 = {
		32.000 = { "flash", 1.000, 0, 0, 1, [unk], [unk], [unk], [unk], [unk] };
	},
	]]
	BPMs = { 0.000 = 280.000 },
	CDTitle = "",
	DisplayBPM = { 150, 300 }, -- can be either an array of two values to cycle or 'Random'
	FormatRevision = 2,
	InstrumentTracks = {
		-- guitar hero/rock band
		Guitar = "guitar.ogg",
		Rhythm = "rhythm.ogg",
		Bass = "bass.ogg",
		-- dj hero
		LeftDeck = "left.ogg",
		RightDeck = "right.ogg",
	},
	Keysounds = {}, -- I forget exactly how the SM format does this, but it'll be similar here.
	LeadOut = 3,
	Warps = { 5 = 4, 10 = 4 },
	LyricsPath = "Dynamite Rave.lrc",
	Offset = 0.014,
	Overlay = {}, -- same as Background, but without path.
	PreviewMusic = { 50.000, 60.000 },
	Selectable = true,
	Stops = { 156.000 = 0.540 },
	TimeSignatures = { 0.000 = "4/4" }
}

local NoteCharts = {
	{
		ChartRevison = 20,
		BPMs = { 0.000 = 140.000 }, -- override
		Description = "Z. Nard", -- usually the chart author or "Copied from *"
		Difficulty = 'Hard', -- "invalid" names will be assumed to be edits.
		EditName = '', -- valid for all charts, typically only shows for edits.
		Meter = '100', -- value clamped between 1 and 100, scaled to whatever works theme-side.
		RadarValues = {
			-- may not be 100%; won't be commented/linebroken in actual file
			Stream = 0.931,
			Voltage = 1.000,
			Air = 0.439,
			Freeze = 0.174,
			Chaos = 0.156,
		},
		Stats = { -- will probably have more i.e. fakes, lifts.
			Total = 645,
			Jumps = 48,
			Holds = 19,
			Mines = 3,
			Hands = 0,
			Rolls = 0
		},
		StepsType = 'Dance_Single',
		Stops = { 140.000 = 0.860 }, -- override
		TimeSignatures = { 0.000 = "4/4" }, -- override
		NoteData = {
			{ -- measure 1
				-- row level
				--{ column level }, works like this:
				{ 0,0,0,0 },
				{ 0,0,0,0 },
				{ 0,0,0,0 },
				{ 0,0,0,1 = { SomeFunction(), 'TNS_GreaterThan', 'TapNoteScore_W5' } } -- example of assigning a function to a note.
			},
			{ -- measure 2
				-- row level
				--{ column level } or in other words
				{ 0,0,0,0 },
				{ 0,0,0,0 },
				{ 0,0,0,0 },
				{ 0,0,0,0 }
			}
		}
	},
	-- second example chart
	{
		ChartRevison = 5,
		BPMs = { 0.000 = 140.000 },
		Description = "B. McLargeHuge",
		Difficulty = 'Expert',
		EditName = '',
		Meter = '60',
		RadarValues = {
			Stream = 0.931,
			Voltage = 1.000,
			Air = 0.439,
			Freeze = 0.174,
			Chaos = 0.156,
		},
		Stats = {
			Total = 16,
			Jumps = 0,
			Holds = 0,
			Mines = 16,
			Hands = 0,
			Rolls = 0
		},
		StepsType = 'Dance_Single',
		NoteData = {
			{ { 1,0,0,0 }, { 0,0,0,0 }, { 0,0,0,0 }, { 0,0,0,'m' }, },
			{ { 0,1,0,0 }, { 0,0,0,0 }, { 0,0,0,0 }, { 0,0,'m',0 }, },
			{ { 0,0,1,0 }, { 0,0,0,0 }, { 0,0,0,0 }, { 0,'m',0,0 }, },
			{ { 0,0,0,1 }, { 0,0,0,0 }, { 0,0,0,0 }, { 'm',0,0,0 }, },
			{ { 0,0,0,0 }, { 1,0,0,0 }, { 0,0,0,'m' }, { 0,0,0,0 }, },
			{ { 0,0,0,0 }, { 0,1,0,0 }, { 0,0,'m',0 }, { 0,0,0,0 }, },
			{ { 0,0,0,0 }, { 0,0,1,0 }, { 0,'m',0,0 }, { 0,0,0,0 }, },
			{ { 0,0,0,0 }, { 0,0,0,1 }, { 'm',0,0,0 }, { 0,0,0,0 }, },
			{ { 0,0,0,0 }, { 0,0,0,'m' }, { 1,0,0,0 }, { 0,0,0,0 }, },
			{ { 0,0,0,0 }, { 0,0,'m',0 }, { 0,1,0,0 }, { 0,0,0,0 }, },
			{ { 0,0,0,0 }, { 0,'m',0,0 }, { 0,0,1,0 }, { 0,0,0,0 }, },
			{ { 0,0,0,0 }, { 'm',0,0,0 }, { 0,0,0,1 }, { 0,0,0,0 }, },
			{ { 0,0,0,'m' }, { 0,0,0,0 }, { 0,0,0,0 }, { 1,0,0,0 }, },
			{ { 0,0,'m',0 }, { 0,0,0,0 }, { 0,0,0,0 }, { 0,1,0,0 }, },
			{ { 0,'m',0,0 }, { 0,0,0,0 }, { 0,0,0,0 }, { 0,0,1,0 }, },
			{ { 'm',0,0,0 }, { 0,0,0,0 }, { 0,0,0,0 }, { 0,0,0,1 }, }
		}
	}
}

local EditCharts = {
	{
		-- todo: uh..... (this is the internal version of the edits.)
		-- should be like NoteData but with fewer available overrides.
	}
}

-- this is a work in progress format. expect things to change as better ideas come about.
-- this isn't how steps currently work internally. that can be dealt with.

--[[
	Below is a minimal example file.
	Just about every value that can be calculated or defaulted to something
	reasonable has been omitted, and much extra whitespace reduced.
]]
MetaTags = {
	Title = {"Black Lawn Finale", "long wait FINALLY"},
	Artist = {"The Flashbulb", "teh falshbald"}
}
MetaData = {
	BannerPath = { "banner.png" },
	Background = { Path = "background.png" },
	BPMs = { 0.000 = 280.000 },
	FormatRevision = 1,
	Offset = 0.028
}
NoteCharts = {
	{
		Description = "S. Slabrock",
		Difficulty = 'Difficulty_Medium',
		Meter = '50',
		StepsType = 'StepsType_Dance_Single',
		NoteData = {
			{{0,0,0,0},{0,0,1,0},{0,0,0,0},{0,0,0,1}}, -- measure of 4ths
			{{0,1,0,0},{0,0,0,0},{0,1,0,0},{0,0,0,1},{0,0,0,0},{0,0,0,1},{0,0,0,0},{0,0,0,1}} -- measure of 8ths
		}
	}
}