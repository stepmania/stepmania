--I'ma call it the fuck you people with fucking huge cdtitles script, With Love - Jousway
local t = Def.ActorFrame {

	Def.Sprite {
		Name="CDTitle";
		InitCommand=cmd(x,SCREEN_CENTER_X-43;y,SCREEN_TOP+210);
		OnCommand=cmd(draworder,106;shadowlength,1;zoom,0.75;diffusealpha,1;zoom,0;bounceend,0.35;zoom,0.75;spin;effectmagnitude,0,180,0);
	};
	
};

local function Update(self)
	local song = GAMESTATE:GetCurrentSong();
	local cdtitle = self:GetChild("CDTitle");
	local height = cdtitle:GetHeight();
	local width = cdtitle:GetWidth();
	
	if song then
		if song:HasCDTitle() then
			cdtitle:visible(true);
			cdtitle:Load(song:GetCDTitlePath());
		else
			cdtitle:visible(false);
		end;
	else
		cdtitle:visible(false);
	end;
	
	if height >= 70 and width >= 70 then
		if height >= width then
		cdtitle:zoom(70/height);
		else
		cdtitle:zoom(70/width);
		end;
	elseif height >= 70 then
		cdtitle:zoom(70/height);
	elseif width >= 70 then
		cdtitle:zoom(70/width);
	else 
		cdtitle:zoom(1);
	end;

end;

t.InitCommand=cmd(SetUpdateFunction,Update);
return t--]]
