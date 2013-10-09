local function StepsDisplay(pn)
	local function set(self, player)
		self:SetFromGameState( player );
	end

	local t = Def.StepsDisplay {
		InitCommand=cmd(Load,"StepsDisplay",GAMESTATE:GetPlayerState(pn););
	};

	if pn == PLAYER_1 then
		t.CurrentStepsP1ChangedMessageCommand=function(self) set(self, pn); end;
		t.CurrentTrailP1ChangedMessageCommand=function(self) set(self, pn); end;
	else
		t.CurrentStepsP2ChangedMessageCommand=function(self) set(self, pn); end;
		t.CurrentTrailP2ChangedMessageCommand=function(self) set(self, pn); end;
	end

	return t;
end
--
local t = LoadFallbackB();
--
for pn in ivalues(PlayerNumber) do
	t[#t+1] = StepsDisplay(pn) .. {
		InitCommand=cmd(x,pn == PLAYER_1 and SCREEN_CENTER_X*0.5 or SCREEN_CENTER_X*1.5;y,SCREEN_BOTTOM-96); 
	};
	
end
--
t[#t+1] = StandardDecorationFromFileOptional("DifficultyIconRow","DifficultyIconRow");
--
return t;
