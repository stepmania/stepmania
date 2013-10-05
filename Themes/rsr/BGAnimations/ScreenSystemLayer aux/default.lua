local bShow = 0;
local t = Def.ActorFrame {};
t[#t+1] = Def.ActorFrame {
	InitCommand=cmd(visible,false);
	ToggleConsoleDisplayMessageCommand=function(self)
		bShow = 1 - bShow;
		self:visible( bShow == 1 );
	end;
	-- Grid
	LoadActor("_8") .. {
		InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;zoomto,SCREEN_WIDTH,SCREEN_HEIGHT;customtexturerect,0,0,SCREEN_WIDTH/8,SCREEN_HEIGHT/8;diffuse,Color.Black;diffusealpha,0.05);
	};
	LoadActor("_16") .. {
		InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;zoomto,SCREEN_WIDTH,SCREEN_HEIGHT;customtexturerect,0,0,SCREEN_WIDTH/16,SCREEN_HEIGHT/16;diffuse,Color.Blue;diffusealpha,0.1);
	};
	LoadActor("_32") .. {
		InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;zoomto,SCREEN_WIDTH,SCREEN_HEIGHT;customtexturerect,0,0,SCREEN_WIDTH/32,SCREEN_HEIGHT/32;diffuse,Color.White;diffusealpha,0.125);
	};
--]]
};
return t;
