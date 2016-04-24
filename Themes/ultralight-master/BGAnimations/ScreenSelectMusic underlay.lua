local bottomBoxSizeX = SCREEN_CENTER_X*0.625;
local bottomBoxSizeY = SCREEN_CENTER_Y*0.2;

local t = Def.ActorFrame{
	-- top section (below the banner); if anything

	-- vertical seperator
	Def.Quad{
		InitCommand=cmd(x,SCREEN_CENTER_X-8;y,SCREEN_CENTER_Y;zoomto,2,SCREEN_HEIGHT;diffusebottomedge,HSV(192,1,0.8););
		OnCommand=cmd(cropbottom,1;sleep,0.2;bouncebegin,0.2;cropbottom,0);
		OffCommand=cmd(linear,0.5;croptop,1);
	};

	
};

return t;