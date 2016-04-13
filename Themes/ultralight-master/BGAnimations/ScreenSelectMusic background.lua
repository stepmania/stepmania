local t = Def.ActorFrame{
	LoadActor(THEME:GetPathB("ScreenWithMenuElements","background"));
	-- this is optional
	--[[
	Def.Quad{
		--SCREEN_WIDTH*0.905
		InitCommand=cmd(x,SCREEN_RIGHT;zoomto,SCREEN_WIDTH,0;y,SCREEN_CENTER_Y;halign,1;valign,0;diffusealpha,0.25;diffuserightedge,HSVA(192,1,1,0.8);shadowlengthy,1.75;shadowcolor,HSVA(204,0.625,0.5,0.5));
		OnCommand=cmd(bounceend,0.5;zoomy,SCREEN_CENTER_Y*1.5;bob;effectmagnitude,0,-15,0;effectperiod,5);
		OffCommand=cmd(bouncebegin,0.325;zoomx,0;);
	};
	--]]
};

return t;