local Players = GAMESTATE:GetHumanPlayers();
--[[ local tMath = {
	PLAYER_1 = -1,
	PLAYER_2 = 1 
}; --]]
--
local t = Def.ActorFrame {};
-- Is Anyone Enabled To Join
t[#t+1] = Def.ActorFrame {
  InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y);
	LoadFont("Common Normal") .. {
		Text="A player has a high score";
		BeginCommand=cmd(visible,SCREENMAN:GetTopScreen():GetAnyEntering());
	};
	LoadFont("Common Normal") .. {
		Text="No players has a high score";
		BeginCommand=cmd(visible,not SCREENMAN:GetTopScreen():GetAnyEntering());
	};
--[[ 	for pn in ivalues(Players) do
		LoadFont("Common Normal") .. {
			Text=ToEnumShortString(pn);
			InitCommand=cmd(x,128 * tMath[pn];y,32);
		};
	end --]]
};
--
t[#t+1] = Def.Actor {
	MenuTimerExpiredMessageCommand = function(self, param)
		for pn in ivalues(PlayerNumber) do
			SCREENMAN:GetTopScreen():Finish(pn);
		end
	end;
	CodeMessageCommand=function(self,param)
		if param.Name == "Enter" then
			SCREENMAN:GetTopScreen():Finish(param.PlayerNumber);
		end
	end
};
--
return t