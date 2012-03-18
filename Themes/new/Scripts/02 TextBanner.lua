local TitleMaxWidth = 228; -- zoom w/subtitle is 0.75 (multiply by 1.25)
local SubtitleMaxWidth = 420; -- zoom is 0.6 (multiply zoom,1 value by 1.4)
local ArtistMaxWidth = 300/0.8;

--[[
-- The old (cmd(blah))(Actor) syntax is hard to read.
-- This is longer, but much easier to read. - Colby
--]]
function TextBannerAfterSet(self,param)
	local Title = self:GetChild("Title")
	local Subtitle = self:GetChild("Subtitle")
	local Artist = self:GetChild("Artist")
	
	if Subtitle:GetText() == "" then
		Title:maxwidth(TitleMaxWidth)
		Title:y(-11)
		Title:zoom(1)
		
		-- hide so that the game skips drawing.
		Subtitle:visible(false)

		Artist:zoom(0.75)
		Artist:maxwidth(ArtistMaxWidth)
		Artist:y(8)
	else
		Title:maxwidth(TitleMaxWidth*1.25)
		Title:y(-14)
		Title:zoom(0.875)
		
		-- subtitle below title
		Subtitle:visible(true)
		Subtitle:zoom(0.625)
		Subtitle:y(0)
		Subtitle:maxwidth(SubtitleMaxWidth)
		
		Artist:zoom(0.6)
		Artist:maxwidth(ArtistMaxWidth)
		Artist:y(12)
	end
end
