local mainMaxWidth = 228; -- zoom w/subtitle is 0.75 (multiply by 1.25)
local subMaxWidth = 420; -- zoom is 0.6 (multiply zoom,1 value by 1.4)
local artistMaxWidth = 300/0.8;

function TextBannerAfterSet(self,param) 
	local Title=self:GetChild("Title"); 
	local Subtitle=self:GetChild("Subtitle"); 
	local Artist=self:GetChild("Artist"); 
	if Subtitle:GetText() == "" then 
		(cmd(maxwidth,mainMaxWidth;y,-8;zoom,1;))(Title);
		(cmd(visible,false))(Subtitle);
		(cmd(zoom,0.66;maxwidth,artistMaxWidth;y,8))(Artist);
	else
		-- subtitle below
		(cmd(maxwidth,mainMaxWidth*1.25;y,-11;zoom,0.75;))(Title);
		(cmd(visible,true;zoom,0.6;y,0;maxwidth,subMaxWidth))(Subtitle); 
		(cmd(zoom,0.6;maxwidth,artistMaxWidth;y,10))(Artist); 
	end
end