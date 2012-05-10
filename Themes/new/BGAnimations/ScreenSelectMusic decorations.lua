local t = LoadFallbackB();
--
t[#t+1] = StandardDecorationFromFileOptional("BannerFrame","BannerFrame");
t[#t+1] = StandardDecorationFromFileOptional("BPMDisplay","BPMDisplay");
t[#t+1] = StandardDecorationFromFileOptional("TimeDisplay","TimeDisplay");
t[#t+1] = StandardDecorationFromFileOptional("StageDisplay","StageDisplay");
t[#t+1] = StandardDecorationFromFileOptional("SortDisplay","SortDisplay");
t[#t+1] = StandardDecorationFromFileOptional("DifficultyList","DifficultyList");

-- StepsDisplay creator
local function CreateStepsDisplay( _pn )
  local function set(self, _pn)
    self:SetFromGameState( _pn);
  end

  local t = Def.StepsDisplay {
    InitCommand=cmd(Load,"StepsDisplay",GAMESTATE:GetPlayerState(_pn););
  };

  if _pn == PLAYER_1 then
    t.CurrentStepsP1ChangedMessageCommand=function(self) set(self, _pn); end;
    t.CurrentTrailP1ChangedMessageCommand=function(self) set(self, _pn); end;
  else
    t.CurrentStepsP2ChangedMessageCommand=function(self) set(self, _pn); end;
    t.CurrentTrailP2ChangedMessageCommand=function(self) set(self, _pn); end;
  end

  return t;
end
-- Create StepsDisplay for each player
for pn in ivalues(PlayerNumber) do
	local MetricsName = "StepsDisplay" .. PlayerNumberToString(pn);
	t[#t+1] = CreateStepsDisplay(pn) .. {
		InitCommand=function(self) self:player(pn); self:name(MetricsName); ActorUtil.LoadAllCommandsAndSetXY(self,Var "LoadingScreen"); end;
	};
end
-- Create PaneDisplay for each player
t[#t+1] = StandardDecorationFromFileOptional("PaneDisplay","PaneDisplay");
--[[ for pn in ivalues(PlayerNumber) do
    local MetricsName = "PaneDisplay" .. PlayerNumberToString(pn);
    t[#t+1] = StandardDecorationFromFileOptional(MetricsName,"PaneDisplay",PlayerNumber);
end ]]
return t;