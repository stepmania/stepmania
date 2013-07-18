local fTileSize = 32;
local iTilesX = math.ceil( SCREEN_WIDTH/fTileSize );
local iTilesY = math.ceil( SCREEN_HEIGHT/fTileSize );
local fSleepTime = THEME:GetMetric( Var "LoadingScreen","ScreenInDelay");
--[[ local function Actor:PositionTile(self,iX,iY)
	self:x( scale(iX,1,iTilesX,-SCREEN_CENTER_X,SCREEN_CENTER_X) );
	self:y( scale(iY,1,iTilesY,-SCREEN_CENTER_Y,SCREEN_CENTER_Y) );
end --]]
local t = Def.ActorFrame {
	InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y);
	OnCommand=cmd(sleep,fSleepTime);
};
--[[ for indx=1,iTilesX do
	for indy=1,iTilesY do
		t[#t+1] = Def.Quad {
			InitCommand=cmd(zoom,fTileSize-2;
				x,math.floor( scale(indx,1,iTilesX,(iTilesX/2)*fTileSize*-1,(iTilesX/2)*fTileSize*1) );
				y,math.floor( scale(indy,1,iTilesY,(iTilesY/2)*fTileSize*-1,(iTilesY/2)*fTileSize*1) );
			);
			OnCommand=cmd(diffuse,Color("Black");diffusealpha,1;zoom,fTileSize-2;sleep,(iTilesX+iTilesY)*(1/60);linear,0.0325 + ( indx / 60 );diffusealpha,0;zoom,fTileSize*1.25);
		};
	end
end --]]
t[#t+1] = Def.Quad {
	InitCommand=cmd(zoomto,SCREEN_WIDTH+1,SCREEN_HEIGHT);
	OnCommand=cmd(diffuse,color("0,0,0,1");sleep,0.0325 + fSleepTime;linear,0.15;diffusealpha,0);
};
--[[ return Def.ActorFrame {
	Def.Quad {
		InitCommand=cmd(Center;zoomto,SCREEN_WIDTH+1,SCREEN_HEIGHT);
		OnCommand=cmd(diffuse,color("0,0,0,1");linear,0.15;diffusealpha,0);
	};
}; --]]

return t