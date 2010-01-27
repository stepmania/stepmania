local t = Def.ActorFrame {};
local x = Def.ActorFrame {
-- 	Name = 'Scroller';
-- 	NumItemsToDraw=10;
-- 	NumItemsToDraw=GAMESTATE:GetCurrentSong():GetStepsSeconds() or GAMESTATE:GetCurrentCourse():GetTotalSeconds(GAMESTATE:GetCurrentSteps(GAMESTATE:GetMasterPlayerNumber()));
-- 	InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y);
-- 	InitCommand=cmd(SetSecondsPauseBetweenItems,0;SetSecondsPerItem,2;ScrollWithPadding,0,0;SetLoop,true;);
-- 	OnCommand=cmd(z,-256;rotationx,-30;spin;effectmagnitude,0,32,0);
-- 	OnCommand=cmd(x,SCREEN_CENTER_X*2;rotationy,30;bob;effectmagnitude,SCREEN_CENTER_X*4,0,0;effectclock,'beat';effectperiod,32);
--[[ 	TransformFunction=function(self, offset, itemIndex, numItems)
		self:x((itemIndex)*SCREEN_WIDTH);
		if itemIndex%2 == 0 then
-- 			self:zoomx(-1);
-- 			self:rotationz(180);
-- 			self:addx(SCREEN_CENTER_X);
-- 			self:addy(-SCREEN_CENTER_Y);
-- 			self:pulse();
		end;
-- 		self:zoom(); 
	end;	--]]
};
x[#x+1] = Def.ActorFrame {
	Def.ActorProxy {
		OnCommand=function(self)
			self:SetTarget( SCREENMAN:GetTopScreen():GetChild('PlayerP1') );
			self:visible(true);
		end;
-- 		OnCommand=cmd(zoom,1);
	};
	Def.ActorProxy {
		OnCommand=function(self)
			self:SetTarget( SCREENMAN:GetTopScreen():GetChild('PlayerP2') );
			self:visible(true);
		end;
-- 		OnCommand=cmd(zoom,1);
	};
};

t[#t+1] = x;
return t