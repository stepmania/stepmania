local player = Var "Player"
local blinkTime = 1.2
local barWidth = 256;
local barHeight = 32;
local c;
local LifeMeter, MaxLives, CurLives;
local LifeRatio;

local t = Def.ActorFrame {
	Def.Quad {
		Name="LifeFill";
		InitCommand=cmd(zoomto,barWidth,barHeight);
		--OnCommand=cmd(diffuseramp;effectcolor2,PlayerColor(player);effectcolor1,ColorMidTone(PlayerColor(player));
		--	effectclock,'beatnooffset';effectperiod,1);
	};
	InitCommand=function(self)
		--c = self:GetChildren();
	end;

	BeginCommand=function(self,param)
		local screen = SCREENMAN:GetTopScreen() or nil;
		if screen ~= nil then 
			LifeMeter = screen:GetLifeMeter(player);
			MaxLives = LifeMeter:GetTotalLives();	
			CurLives = LifeMeter:GetLivesLeft();
			MESSAGEMAN:Broadcast("SystemMessage",
				{ Message = "(" .. MaxLives .. ", " .. CurLives ")", NoAnimate = true });
		else
			MESSAGEMAN:Broadcast("SystemMessage", { Message = "WE DONE FUCKED UP"});
		end
	end;
	LifeChangedMessageCommand=function(self,param)
		if param.Player == player then
			LifeRatio = CurLives / MaxLives;
		end
	end;
	StartCommand=function(self,param)
		if param.Player == player then
			LifeRatio = CurLives / MaxLives;
		end
	end;
	FinishCommand=function(self,param)
		if param.Player == player then
			LifeRatio = CurLives / MaxLives;
		end
	end;
	--[[ LoadActor("_lives")..{
		InitCommand=cmd(pause;horizalign,left;x,-(barWidth/2));
		BeginCommand=function(self,param)
			local screen = SCREENMAN:GetTopScreen();
			local glifemeter = screen:GetLifeMeter(player);
				self:setstate(glifemeter:GetTotalLives()-1);
				
				if glifemeter:GetTotalLives() <= 4 then
					self:zoomx(barWidth/(4*64));
				else
					self:zoomx(barWidth/((glifemeter:GetTotalLives())*64));
				end
				self:cropright((640-(((glifemeter:GetTotalLives())*64)))/640);
		end;
		LifeChangedMessageCommand=function(self,param)
			if param.Player == player then
				if param.LivesLeft == 0 then
					self:visible(false)
				else
					self:setstate( math.max(param.LivesLeft-1,0) )
					self:visible(true)
				end
			end
		end;
		StartCommand=function(self,param)
			if param.Player == player then
				self:setstate( math.max(param.LivesLeft-1,0) );			
			end			
		end;
		FinishCommand=cmd(playcommand,"Start");
	}; ]]
	
};

return t;