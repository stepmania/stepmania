local player = Var "Player"
local blinkTime = 1.2
local barWidth = 256;
local barHeight = 32;
local c

local function CreateLives(numLives)
	local t = {};
	for i=1,numLives do
		t[#t+1] = Def.Quad {
			Name=i;
			OnCommand=cmd(zoom,1024);
		};
	end
	return t
end

local t = Def.ActorFrame {
	Def.ActorFrame {
		Name="LifeContainer";
	};
	InitCommand=function(self)
		c = self:GetChildren();
	end;

	BeginCommand=function(self,param)
		MESSAGEMAN:Broadcast("SystemMessage",{ Message = "TEST", NoAnimate = true});
		local c = self:GetChildren();
		local screen = SCREENMAN:GetTopScreen();
		local lifemeter = screen:GetLifeMeter(player);
		MESSAGEMAN:Broadcast("SystemMessage",{ Message = "TEST2", NoAnimate = true});
		local TotalLives = lifemeter:GetTotalLives();	
		local CurLives = lifemeter:GetLivesLeft();
		MESSAGEMAN:Broadcast("SystemMessage",{ Message = "ASSIGN CONTAINERS", NoAnimate = true});
		c.LifeContainer[#c.LifeContainer+1] = CreateLives(TotalLives);
		MESSAGEMAN:Broadcast("SystemMessage",{ Message = "LIVES = " .. TotalLives .. " | CUR = " .. CurLives .. " | ACTORS: " .. self:GetNumChildren(), NoAnimate = true});
	end;
	LifeChangedMessageCommand=function(self,param)
		if param.Player == player then
			return
		end
	end;
	StartCommand=function(self,param)
		if param.Player == player then
			return
		end
	end;
	FinishCommand=function(self,param)
		if param.Player == player then
			return 
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