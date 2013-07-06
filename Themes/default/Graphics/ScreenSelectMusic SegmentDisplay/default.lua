-- segment display: tells the player about various gimmicks used in the song timing.
local iconPath = "_timingicons"
local leftColX = -144
local rightColX = -leftColX

local showCmd = function(self)
	self:stoptweening();
	self:accelerate(0.1);
	self:diffusealpha(1);
end;
local hideCmd = function(self)
	self:stoptweening();
	self:accelerate(0.1);
	self:diffusealpha(0);
end;

local SegmentTypes = {
	Stops	=	{ Frame = 0, xPos = leftColX, yPos = 0 },
	Warps	=	{ Frame = 2, xPos = leftColX, yPos = -16 },
	Delays	=	{ Frame = 1, xPos = leftColX, yPos = -32 },
	Attacks	=	{ Frame = 6, xPos = leftColX, yPos = 16 },
	Scrolls	=	{ Frame = 3, xPos = rightColX, yPos = -32 },
	Speeds	=	{ Frame = 4, xPos = rightColX, yPos = -17 },
	Fakes	=	{ Frame = 5, xPos = rightColX, yPos = -2 },
};

local t = Def.ActorFrame{
	BeginCommand=function(self)
		self:playcommand("SetIcons");
		self:playcommand("SetAttacksIconMessage");
	end;

	SetIconsCommand=function(self)
		local stops = self:GetChild("StopsIcon")
		local delays = self:GetChild("DelaysIcon")
		local warps = self:GetChild("WarpsIcon")
		local scrolls = self:GetChild("ScrollsIcon")
		local speeds = self:GetChild("SpeedsIcon")
		local fakes = self:GetChild("FakesIcon")

		-- hax
		MESSAGEMAN:Broadcast("SetAttacksIcon",{Player = GAMESTATE:GetMasterPlayerNumber()})

		local song = GAMESTATE:GetCurrentSong()
		if song then
			local timing = song:GetTimingData()

			if timing:HasWarps() then warps:playcommand("Show")
			else warps:playcommand("Hide")
			end

			if timing:HasStops() then stops:playcommand("Show")
			else stops:playcommand("Hide")
			end

			if timing:HasDelays() then delays:playcommand("Show")
			else delays:playcommand("Hide")
			end

			if timing:HasScrollChanges() then scrolls:playcommand("Show")
			else scrolls:playcommand("Hide")
			end

			if timing:HasSpeedChanges() then speeds:playcommand("Show")
			else speeds:playcommand("Hide")
			end

			if timing:HasFakes() then fakes:playcommand("Show")
			else fakes:playcommand("Hide")
			end
		else
			warps:playcommand("Hide")
			stops:playcommand("Hide")
			delays:playcommand("Hide")
			scrolls:playcommand("Hide")
			speeds:playcommand("Hide")
			fakes:playcommand("Hide")
		end
	end;
	SetAttacksIconMessageCommand=function(self,param)
		local attacks = self:GetChild("AttacksIcon")
		local song = GAMESTATE:GetCurrentSong()
		if song then
			local steps = GAMESTATE:GetCurrentSteps(param.Player)
			if steps then
				local hasAttacks = steps:HasAttacks()
				attacks:playcommand(hasAttacks and "Show" or "Hide")
			else
				attacks:playcommand("Hide")
			end
		else
			attacks:playcommand("Hide")
		end
	end;

	LoadActor(iconPath)..{
		Name="WarpsIcon";
		InitCommand=function(self)
			self:animate(false);
			self:x(SegmentTypes.Warps.xPos);
			self:y(SegmentTypes.Warps.yPos);
			self:setstate(SegmentTypes.Warps.Frame);
			self:diffusealpha(0);
		end;
		ShowCommand=showCmd;
		HideCommand=hideCmd;
	};
	LoadActor(iconPath)..{
		Name="StopsIcon";
		InitCommand=function(self)
			self:animate(false);
			self:x(SegmentTypes.Stops.xPos);
			self:y(SegmentTypes.Stops.yPos);
			self:setstate(SegmentTypes.Stops.Frame);
			self:diffusealpha(0);
		end;
		ShowCommand=showCmd;
		HideCommand=hideCmd;
	};
	LoadActor(iconPath)..{
		Name="DelaysIcon";
		InitCommand=function(self)
			self:animate(false);
			self:x(SegmentTypes.Delays.xPos);
			self:y(SegmentTypes.Delays.yPos);
			self:setstate(SegmentTypes.Delays.Frame);
			self:diffusealpha(0);
		end;
		ShowCommand=showCmd;
		HideCommand=hideCmd;
	};
	LoadActor(iconPath)..{
		Name="AttacksIcon";
		InitCommand=function(self)
			self:animate(false);
			self:x(SegmentTypes.Attacks.xPos);
			self:y(SegmentTypes.Attacks.yPos);
			self:setstate(SegmentTypes.Attacks.Frame);
			self:diffusealpha(0);
		end;
		ShowCommand=showCmd;
		HideCommand=hideCmd;
	};
	LoadActor(iconPath)..{
		Name="ScrollsIcon";
		InitCommand=function(self)
			self:animate(false);
			self:x(SegmentTypes.Scrolls.xPos);
			self:y(SegmentTypes.Scrolls.yPos);
			self:setstate(SegmentTypes.Scrolls.Frame);
			self:diffusealpha(0);
		end;
		ShowCommand=showCmd;
		HideCommand=hideCmd;
	};
	LoadActor(iconPath)..{
		Name="SpeedsIcon";
		InitCommand=function(self)
			self:animate(false);
			self:x(SegmentTypes.Speeds.xPos);
			self:y(SegmentTypes.Speeds.yPos);
			self:setstate(SegmentTypes.Speeds.Frame);
			self:diffusealpha(0);
		end;
		ShowCommand=showCmd;
		HideCommand=hideCmd;
	};
	LoadActor(iconPath)..{
		Name="FakesIcon";
		function(self)
			self:animate(false);
			self:x(SegmentTypes.Fakes.xPos);
			self:y(SegmentTypes.Fakes.yPos);
			self:setstate(SegmentTypes.Fales.Frame);
			self:diffusealpha(0);
		end;
		ShowCommand=showCmd;
		HideCommand=hideCmd;
	};
	CurrentSongChangedMessageCommand=function(self)
		self:playcommand("SetIcons");
	end;
	CurrentStepsP1ChangedMessageCommand=function(self) MESSAGEMAN:Broadcast("SetAttacksIcon",{Player = PLAYER_1}) end;
	CurrentStepsP2ChangedMessageCommand=function(self) MESSAGEMAN:Broadcast("SetAttacksIcon",{Player = PLAYER_2}) end;
};

return t;