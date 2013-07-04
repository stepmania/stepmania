local Players = GAMESTATE:GetHumanPlayers();
--[[ local tMath = {
	PLAYER_1 = -1,
	PLAYER_2 = 1 
}; --]]
--
local t = Def.ActorFrame {};
-- Is Anyone Enabled To Join
t[#t+1] = Def.ActorFrame {
	InitCommand=function(self)
		self:x(SCREEN_CENTER_X);
		self:y(SCREEN_CENTER_Y);
	end;
	LoadFont("Common Normal") .. {
		Text="A player has a high score";
		BeginCommand=function(self)
			self:visible(SCREENMAN:GetTopScreen():GetAnyEntering());
		end;
	};
	LoadFont("Common Normal") .. {
		Text="No players has a high score";
		BeginCommand=function(self)
			self:visible(not SCREENMAN:GetTopScreen():GetAnyEntering());
		end;
	};
--[[ 	
	for pn in ivalues(Players) do
		LoadFont("Common Normal") .. {
			Text=ToEnumShortString(pn);
			InitCommand=function(self)
				self:x(128 * tMath[pn]);
				self:y(32);
			end
		};
	end
--]]
};
--
t[#t+1] = Def.Actor {
	MenuTimerExpiredMessageCommand = function(self, param)
		for pn in ivalues(Players) do
			SCREENMAN:GetTopScreen():Finish(pn);
		end
	end;
	CodeMessageCommand=function(self,param)
		if param.Name == "Enter" then
			SCREENMAN:GetTopScreen():Finish(pn);
		end
	end
};
--
return t