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
