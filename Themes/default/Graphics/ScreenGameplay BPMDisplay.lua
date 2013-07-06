-- check if players are playing steps with different timingdata.
local numPlayers = GAMESTATE:GetNumPlayersEnabled()

local function UpdateSingleBPM(self)
	local bpmDisplay = self:GetChild("BPMDisplay")
	local pn = GAMESTATE:GetMasterPlayerNumber()
	local pState = GAMESTATE:GetPlayerState(pn);
	local songPosition = pState:GetSongPosition()
	local bpm = songPosition:GetCurBPS() * 60
	bpmDisplay:settext( string.format("%03.2f",bpm) )
end

local displaySingle = Def.ActorFrame{
	-- manual bpm display
	LoadFont("BPMDisplay", "bpm")..{
		Name="BPMDisplay";
		InitCommand=function(self)
			self:zoom(0.675);
			self:shadowlength(1);
		end;
	};
};

displaySingle.InitCommand=function(self)
	self:SetUpdateFunction(UpdateSingleBPM);
end;

if numPlayers == 1 then
	return displaySingle
else
	-- check if both players are playing the same steps
	local stepsP1 = GAMESTATE:GetCurrentSteps(PLAYER_1)
	local stepsP2 = GAMESTATE:GetCurrentSteps(PLAYER_2)

	if not stepsP1 or not stepsP2 then
		return displaySingle
	end

	local stP1 = stepsP1:GetStepsType()
	local stP2 = stepsP2:GetStepsType()

	local diffP1 = stepsP1:GetDifficulty()
	local diffP2 = stepsP2:GetDifficulty()

	-- get timing data...
	local timingP1 = stepsP1:GetTimingData()
	local timingP2 = stepsP2:GetTimingData()

	--if stP1 == stP2 and diffP1 == diffP2 then
	if timingP1 == timingP2 then
		-- both players are steps with the same TimingData; only need one.
		return displaySingle
	end

	-- otherwise, we have some more work to do.

	local function Update2PBPM(self)
		local dispP1 = self:GetChild("DisplayP1")
		local dispP2 = self:GetChild("DisplayP2")

		-- needs current bpm for p1 and p2
		for pn in ivalues(PlayerNumber) do
			local bpmDisplay = (pn == PLAYER_1) and dispP1 or dispP2
			local pState = GAMESTATE:GetPlayerState(pn);
			local songPosition = pState:GetSongPosition()
			local bpm = songPosition:GetCurBPS() * 60
			bpmDisplay:settext( string.format("%03.2f",bpm) )
		end
	end

	local playerOffset = 36 -- was 28
	local displayTwoPlayers = Def.ActorFrame{
		-- manual bpm displays
		LoadFont("BPMDisplay", "bpm")..{
			Name="DisplayP1";
			InitCommand=function(self)
				self:x(-playerOffset);
				self:zoom(0.6);
				self:shadowlength(1);
			end;
		};
		LoadFont("BPMDisplay", "bpm")..{
			Name="DisplayP2";
			InitCommand=function(self)
				self:x(playerOffset);
				self:zoom(0.6);
				self:shadowlength(1);
			end;
		};
	};

	displayTwoPlayers.InitCommand=function(self)
		self:SetUpdateFunction(Update2PBPM);
	end;

	return displayTwoPlayers
end

-- should not get here
-- return Def.ActorFrame{}