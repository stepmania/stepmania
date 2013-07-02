local mainMaxWidth = 228; -- zoom w/subtitle is 0.75 (multiply by 1.25)
local subMaxWidth = 420; -- zoom is 0.6 (multiply zoom,1 value by 1.4)
local artistMaxWidth = 300/0.8;

function TextBannerAfterSet(self,param) 
	local Title=self:GetChild("Title"); 
	local Subtitle=self:GetChild("Subtitle"); 
	local Artist=self:GetChild("Artist"); 
	if Subtitle:GetText() == "" then 
		Title:maxwidth(mainMaxWidth);
		Title:y(-8);
		Title:zoom(1);
		Subtitle:visible(false);
		Artist:zoom(0.66);
		Artist:maxwidth(artistMaxWidth);
		Artist:y(8);
	else
		-- subtitle below
		Title:maxwidth(mainMaxWidth * 1.25);
		Title:y(-11);
		Title:zoom(0.75);
		Subtitle:visible(true);
		Subtitle:zoom(0.6);
		Subtitle:y(0);
		Subtitle:maxwidth(subMaxWidth);
		Artist:zoom(0.6);
		Artist:maxwidth(artistMaxWidth);
		Artist:y(10);
	end
end