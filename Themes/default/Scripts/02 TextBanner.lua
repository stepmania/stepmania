local mainMaxWidth = 228; -- zoom w/subtitle is 0.75 (multiply by 1.25)
local subMaxWidth = 420; -- zoom is 0.6 (multiply zoom,1 value by 1.4)
local artistMaxWidth = 300/0.8;

local mainMaxWidthHighScore = 192; -- zoom w/subtitle is 0.75 (multiply by 1.25)
local subMaxWidthHighScore = 280; -- zoom is 0.6 (multiply zoom,1 value by 1.4)
local artistMaxWidthHighScore = 280/0.8;

--[[
-- The old (cmd(blah))(Actor) syntax is hard to read.
-- This is longer, but much easier to read. - Colby
--]]
function TextBannerAfterSet(self,param)
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
		Subtitle:zoom(0.6)
		Subtitle:y(0)
		Subtitle:maxwidth(subMaxWidth)
		
		Artist:zoom(0.6)
		Artist:maxwidth(artistMaxWidth)
		Artist:y(10)
	end
end

function TextBannerHighScoreAfterSet(self,param)
	local Title = self:GetChild("Title")
	local Subtitle = self:GetChild("Subtitle")
	local Artist = self:GetChild("Artist")
	
	if Subtitle:GetText() == "" then
		Title:maxwidth(mainMaxWidthHighScore)
		Title:y(-8)
		Title:zoom(1)
		
		-- hide so that the game skips drawing.
		Subtitle:visible(false)

		Artist:zoom(0.66)
		Artist:maxwidth(artistMaxWidthHighScore)
		Artist:y(8)
	else
		Title:maxwidth(mainMaxWidthHighScore*1.25)
		Title:y(-11)
		Title:zoom(0.75)
		
		-- subtitle below title
		Subtitle:visible(true)
		Subtitle:zoom(0.6)
		Subtitle:y(0)
		Subtitle:maxwidth(subMaxWidthHighScore)
		
		Artist:zoom(0.6)
		Artist:maxwidth(artistMaxWidthHighScore)
		Artist:y(10)
	end
end
