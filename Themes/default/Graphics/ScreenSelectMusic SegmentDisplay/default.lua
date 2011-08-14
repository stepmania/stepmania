-- segment display: tells the player about various gimmicks used in the song timing.
local iconPath = "_timingicons"
local leftColX = -144
local rightColX = -leftColX

local showCmd = cmd(stoptweening;accelerate,0.125;diffusealpha,1)
local hideCmd = cmd(stoptweening;accelerate,0.125;diffusealpha,0)

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
	BeginCommand=cmd(playcommand,"SetIcons";playcommand,"SetAttacksIconMessage");
	OffCommand=cmd( RunCommandsOnChildren,cmd(playcommand,"Hide") );

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
		InitCommand=cmd(animate,false;x,SegmentTypes.Warps.xPos;y,SegmentTypes.Warps.yPos;setstate,SegmentTypes.Warps.Frame;diffusealpha,0);
		ShowCommand=showCmd;
		HideCommand=hideCmd;
	};
	LoadActor(iconPath)..{
		Name="StopsIcon";
		InitCommand=cmd(animate,false;x,SegmentTypes.Stops.xPos;y,SegmentTypes.Stops.yPos;setstate,SegmentTypes.Stops.Frame;diffusealpha,0);
		ShowCommand=showCmd;
		HideCommand=hideCmd;
	};
	LoadActor(iconPath)..{
		Name="DelaysIcon";
		InitCommand=cmd(animate,false;x,SegmentTypes.Delays.xPos;y,SegmentTypes.Delays.yPos;setstate,SegmentTypes.Delays.Frame;diffusealpha,0);
		ShowCommand=showCmd;
		HideCommand=hideCmd;
	};
	LoadActor(iconPath)..{
		Name="AttacksIcon";
		InitCommand=cmd(animate,false;x,SegmentTypes.Attacks.xPos;y,SegmentTypes.Attacks.yPos;setstate,SegmentTypes.Attacks.Frame;diffusealpha,0);
		ShowCommand=showCmd;
		HideCommand=hideCmd;
	};
	LoadActor(iconPath)..{
		Name="ScrollsIcon";
		InitCommand=cmd(animate,false;x,SegmentTypes.Scrolls.xPos;y,SegmentTypes.Scrolls.yPos;setstate,SegmentTypes.Scrolls.Frame;diffusealpha,0);
		ShowCommand=showCmd;
		HideCommand=hideCmd;
	};
	LoadActor(iconPath)..{
		Name="SpeedsIcon";
		InitCommand=cmd(animate,false;x,SegmentTypes.Speeds.xPos;y,SegmentTypes.Speeds.yPos;setstate,SegmentTypes.Speeds.Frame;diffusealpha,0);
		ShowCommand=showCmd;
		HideCommand=hideCmd;
	};
	LoadActor(iconPath)..{
		Name="FakesIcon";
		InitCommand=cmd(animate,false;x,SegmentTypes.Fakes.xPos;y,SegmentTypes.Fakes.yPos;setstate,SegmentTypes.Fakes.Frame;diffusealpha,0);
		ShowCommand=showCmd;
		HideCommand=hideCmd;
	};
	CurrentSongChangedMessageCommand=cmd(playcommand,"SetIcons";);
	CurrentStepsP1ChangedMessageCommand=function(self) MESSAGEMAN:Broadcast("SetAttacksIcon",{Player = PLAYER_1}) end;
	CurrentStepsP2ChangedMessageCommand=function(self) MESSAGEMAN:Broadcast("SetAttacksIcon",{Player = PLAYER_2}) end;
};

return t;