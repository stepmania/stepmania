local fTileSize = 32;
local iTilesX = math.ceil( SCREEN_WIDTH/fTileSize );
local iTilesY = math.ceil( SCREEN_HEIGHT/fTileSize );
local fSleepTime = THEME:GetMetric( Var "LoadingScreen","ScreenOutDelay");

local t = Def.ActorFrame {
	InitCommand=function(self)
		self:x(SCREEN_CENTER_X);
		self:y(SCREEN_CENTER_Y);
	end;
	OnCommand=function(self)
		self:sleep(fSleepTime);
	end;
};

t[#t+1] = Def.Quad {
	InitCommand=function(self)
		self:zoomto(SCREEN_WIDTH + 1, SCREEN_HEIGHT);
	end;
	OnCommand=function(self)
		self:diffuse(color("0,0,0,0"));
		self:sleep(0.0325 + fSleepTime);
		self:linear(0.15);
		self:diffusealpha(0);
	end;
};


return t