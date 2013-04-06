local t = Def.ActorFrame {
	OnCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_BOTTOM-96;zoomy,0;sleep,0.5;decelerate,0.25;zoomy,1);
	OffCommand=cmd(bouncebegin,0.15;zoomx,0);
	LoadActor(THEME:GetPathG("CDTitle", "Con")) .. {
		InitCommand=cmd(diffuse,Color("Orange"));
	};
	LoadFont("Common Normal") .. {
		Text="Author";
		InitCommand=cmd(zoom,0.7;y,-36,diffuse,0,0,0,1;shadowlength,1); -- strokecolor,Color("Black")
	};
	Def.Sprite {
		Name="CDTitle";
		InitCommand=cmd(y,19);
		--OnCommand=cmd(draworder,106;shadowlength,1;zoom,0.75;diffusealpha,1;zoom,0;bounceend,0.35;zoom,0.75;spin;effectmagnitude,0,180,0);
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
	
	if height >= 60 and width >= 80 then
		if height+20 >= width then
		cdtitle:zoom(60/height);
		else
		cdtitle:zoom(80/width);
		end;
	elseif height >= 60 then
		cdtitle:zoom(60/height);
	elseif width >= 80 then
		cdtitle:zoom(80/width);
	else 
		cdtitle:zoom(1);
	end;

end;

t.InitCommand=cmd(SetUpdateFunction,Update);
return t--]]
