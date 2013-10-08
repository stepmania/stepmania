Theme = {}

Theme.MusicWheelItemWidth = function()
	return tonumber(THEME:GetMetric("MusicWheel", "ItemWidth"))
end

Theme.MusicWheelTitleSet = function(self, params)
	local song = params.Song
	if not song then return end
	local width = Theme.MusicWheelItemWidth()-20
	local title = song:GetDisplayFullTitle()
	local artist = song:GetDisplayArtist()
	if title == "Let It Flow" and artist == "BANTU" then
		self:jitter(true)
	else
		self:jitter(false)
	end
	if title:find("Katamari") then
		self:rainbowscroll(true)
	else
		self:rainbowscroll(false)
	end
	if title == "KING of the SEA" then
		self:zoom(1.5)
		self:maxwidth(width*(1/1.5))
	end
	if (title:lower():find("hardstyle") or title:lower():find("hardcore")) and
		params.HasFocus then
		self:diffuseramp()
		self:effectclock("bgm")
		self:effectcolor1(color("#ccaaaa"))
	else
		self:stopeffect()
	end
	if artist == "Angerfist" and params.HasFocus then
		self:diffuseramp()
		self:effectclock("bgm")
		self:effectcolor1(color("#ffaaaa"))
	else
		self:stopeffect()
	end
end

Theme.MusicWheelSubtitleSet=function(self, params)
	local song = params.Song
	if not song then return end

	-- I've got a few simfiles here with an annoying prefix here.
	local subtitle = song:GetDisplaySubTitle()
	if subtitle:sub(1, 2) == "- " then
		self:settext(subtitle:sub(3))
	end
end

Theme.MusicWheelArtistSet=function(self, params)
	self:diffuse(color("#aaaaaa"))
end

Theme.MusicWheelAfterSet=function(self, params)
	local width = Theme.MusicWheelItemWidth()-20
	local mainMaxWidth = width
	local subMaxWidth = width*(1/0.55)
	local artistMaxWidth = width*(1/0.66)
	local Title = self:GetChild("Title")
	local Subtitle = self:GetChild("Subtitle")
	local Artist = self:GetChild("Artist")
	
	if Subtitle:GetText() == "" then
		Title:maxwidth(mainMaxWidth)
		Title:y(-8)
		Title:zoom(1)
		
		-- hide so that the game skips drawing.
		Subtitle:visible(false)
		
		Artist:zoom(0.66)
		Artist:maxwidth(artistMaxWidth)
		Artist:y(8)
	else
		Title:maxwidth(mainMaxWidth*1.25)
		Title:y(-11)
		Title:zoom(0.75)
		
		-- subtitle below title
		Subtitle:visible(true)
		Subtitle:zoom(0.55)
		Subtitle:y(0)
		Subtitle:maxwidth(subMaxWidth)
		
		Artist:zoom(0.6)
		Artist:maxwidth(artistMaxWidth)
		Artist:y(10)
	end
end


-- super useful for making code less redundant!
Theme.Roundbox = function(Width, Height, Color)
	-- code adapted from shakesoda's optical
	assert(Width)
	assert(Height)
	local corner = THEME:GetPathG("Common","corner") -- graphic file
	local DefaultColor = color("0,0,0,0.75") -- black box omg magic

	-- Color is optional.
	if not Color then Color = DefaultColor end

	--[[
	How it's drawn:
	  c----c
	  OOOOOO
	  c----c

	---- is 8px tall and Width-8 wide. y = (Height/2), flip the bit.
	OOOO is Height-8px tall and Width wide.
	c's x position is Width - 4, flip the bit if needed.
	--]]
	local EdgeWidth = Width-8
	local EdgePosY = (Height/2)
	local CornerPosX = ((Width/2)-4)

	return Def.ActorFrame {
		BeginCommand=cmd(runcommandsonleaves,cmd(diffuse,Color)),
		-- top
		Def.Quad { InitCommand=cmd(zoomto,EdgeWidth-8,8;y,-EdgePosY) },
		-- middle
		Def.Quad { InitCommand=cmd(zoomto,Width,Height-8) },
		-- bottom
		Def.Quad { InitCommand=cmd(zoomto,EdgeWidth-8,8;y,EdgePosY) },
		-- top left
		LoadActor(corner)..{ InitCommand=cmd(x,-CornerPosX;y,-EdgePosY) },
		-- top right
		LoadActor(corner)..{ InitCommand=cmd(x,CornerPosX;y,-EdgePosY;rotationz,90) },
		-- bottom left
		LoadActor(corner)..{ InitCommand=cmd(x,-CornerPosX;y,EdgePosY;rotationz,-90) },
		-- bottom right
		LoadActor(corner)..{ InitCommand=cmd(x,CornerPosX;y,EdgePosY;rotationz,180) }
	}
end


-- super useful for making code less redundant!
Theme.RoundboxInverse = function(Width, Height, Color)
	-- code adapted from shakesoda's optical
	assert(Width)
	assert(Height)
	local corner = THEME:GetPathG("Common","corner") -- graphic file
	local DefaultColor = color("0,0,0,0.75") -- black box omg magic

	-- Color is optional.
	if not Color then Color = DefaultColor end

	--[[
	How it's drawn:
	  c----c
	  OOOOOO
	  c----c

	---- is 8px tall and Width-8 wide. y = (Height/2), flip the bit.
	OOOO is Height-8px tall and Width wide.
	c's x position is Width - 4, flip the bit if needed.
	--]]
	local EdgeWidth = Width-8
	local EdgePosY = (Height/2)
	local CornerPosX = ((Width/2)-4)

	return Def.ActorFrame {
		BeginCommand=cmd(runcommandsonleaves,cmd(diffuse,Color)),
		-- top left
		LoadActor(corner)..{ InitCommand=cmd(x,-CornerPosX;y,-EdgePosY;rotationz,180) },
		-- top right
		LoadActor(corner)..{ InitCommand=cmd(x,CornerPosX;y,-EdgePosY;rotationz,270) },
		-- bottom left
		LoadActor(corner)..{ InitCommand=cmd(x,-CornerPosX;y,EdgePosY;rotationz,-270) },
		-- bottom right
		LoadActor(corner)..{ InitCommand=cmd(x,CornerPosX;y,EdgePosY;rotationz,0) }
	}
end

Theme.GetAspect = function()
	return SCREEN_WIDTH / SCREEN_HEIGHT
end