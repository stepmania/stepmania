local t = Def.ActorFrame{ FOV=45; };
t[#t+1] = LoadActor( THEME:GetPathB("","_titlebg") )..{
	InitCommand=cmd(Center;zoomto,SCREEN_WIDTH,SCREEN_WIDTH;);
};
t[#t+1] = Def.Quad{
	InitCommand=cmd(Center;zoomto,SCREEN_WIDTH,SCREEN_HEIGHT;diffusealpha,0.2);
	--OnCommand=cmd(diffuse,color("0.65,0.35,0,0.35"));
	OnCommand=cmd(diffuse,color("0,0.5,0.75,0.35"));
};
t[#t+1] = Def.Quad{
	InitCommand=cmd(CenterX;y,SCREEN_TOP;vertalign,top;zoomto,SCREEN_WIDTH,SCREEN_HEIGHT*0.25;);
	OnCommand=cmd(fadebottom,0.5;diffuse,color("0.0625,0.15625,0.28175,1"));
};

-- stars
local numStars= 250;
local maxZ = SCREEN_WIDTH;
local zStep = 5;
local reverse = false;
local zSpeed = 0.025; -- was 0.1

for i=1,numStars do
	local starSize = math.random(0.25,4);
	local randX = math.random(SCREEN_CENTER_X*0.05,SCREEN_CENTER_X*1.95);
	local randY = math.random(SCREEN_CENTER_Y*0.05,SCREEN_CENTER_Y*1.95);
	local fxOffset = math.random(0,15);
	local fxPeriod = math.random(5,60);
	t[#t+1] = Def.Quad{
		InitCommand=cmd(x,randX;y,randY;zoomto,starSize,starSize;blend,bmAdd);
		OnCommand=cmd(diffuseshift;effectperiod,fxPeriod;effectoffset,fxOffset;effectcolor1,color("1,1,1,0.25");effectcolor2,color("1,1,1,0.5");queuecommand,'Zoomer');
		ZoomerCommand=function(self)
			-- check Z
			--[[
			if self:GetZ()-starSize > maxZ then
				self:linear(0.2);
				self:diffusealpha(0);
				self:sleep(0.001);
				self:z(0);
				self:linear(0.2);
				self:diffusealpha(1);
			end;
			--]]
			if self:GetZ()-starSize > maxZ then
				reverse = true;
			end;
			if self:GetZ() == 0 then
				reverse = false;
			end;

			local multi = 1;
			if reverse then multi = -1; end;

			self:linear(zSpeed);
			self:addz(zStep * multi);
			self:queuecommand('Zoomer');
		end;
		cmd(linear,starSize*20;addz,200;);
	};
end;

return t;