local player = Var "Player"
local blinkTime = 1.2

local t = Def.ActorFrame{
	LoadActor("_lives")..{
		-- xxx: assumes course is 4lives
		InitCommand=cmd(pause;setstate,3);
		LifeChangedMessageCommand=function(self,param)
			if param.Player == player then
				if param.LivesLeft <= 4 then
					self:setstate( math.max(param.LivesLeft-1,0) )
				else
					self:setstate(3)
				end
			end
		end;
	};
};

return t;