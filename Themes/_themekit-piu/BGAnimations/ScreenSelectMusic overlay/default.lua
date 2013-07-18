--Whew... this one took some time...
local function ModIcons(p)
	local oldmods
	
	--todo: add more
	local noteskinbuttons = {
		pump = "UpLeft";
		dance ="Down";
	};
	local function ModiconRow(caption)
		return Def.ActorFrame {
			Draw.RoundBox(35,35,6,6,color("#32ff00"));
			Draw.RoundBox(25,25,3,3,color("#32c800"));
			LoadFont("_arial black")..{
				Name="caption";
				Text=caption or "";
			};
			LoadActor("i can flash")..{
				Name="sfx";
				InitCommand=cmd(blend,Blend.Add;zoom,0.125);
				SFXCommand=cmd(stoptweening;diffusealpha,1;zoom,0.5;linear,0.2;zoom,1;diffusealpha,0);
			};
		}
	end
	--ActorScrollers <3
	return Def.ActorScroller {
		--TransformFunctions are both love and hate...
		TransformFunction=function(self, offset, itemIndex, numItems)
			self:y(offset * 40);
		end;
		InitCommand=function(self)
			local items = self:GetNumChildren()
			self:SetCurrentAndDestinationItem(items/2);
			if SSC then
				self:SetNumItemsToDraw(items);
			end
			self:SetLoop(true);
			self:playcommand("Set");
		end;
		PlayerOptionsChangedMessageCommand=cmd(playcommand,"Set");
		PlayerJoinedMessageCommand=cmd(playcommand,"Set");
		--Here's where the magic happens
		SetCommand=function(self)
			local playermods = GAMESTATE:GetPlayerState(p):GetPlayerOptionsArray('ModsLevel_Preferred')
			--clean (invisible everything)
			for k,v in pairs(self:GetChildren()) do
				self:GetChild(k):visible(false);
			end
			--stop early
			if not GAMESTATE:IsHumanPlayer(p) then return end;
			--this thing is based on iteration... ugh
			for k,v in ipairs(playermods) do
				--check if it's speed
				if string.find(v,"%dx") ~= nil then
					local c = self:GetChild("%dx")
					c:visible(true);
					c:GetChild("caption"):settext(v);
					c:GetChild("sfx"):playcommand("SFX");
				else
					--check if it's noteskin
					local noteskins = NOTESKIN:GetNoteSkinNames()
					for i,n in ipairs(noteskins) do
						if n == v then
							local c = self:GetChild("Noteskin")
							local button = noteskinbuttons[sGame] or "fallback"
							local toload = NOTESKIN:GetPathForNoteSkin(button, "Tap Note", v)
							--make it visible
							c:visible(true);
							--adjust sizes here
							c:GetChild("skingraphic"):Load(toload);
							c:GetChild("skingraphic"):zoom(0.45);
							--effects
							c:GetChild("skinframe"):GetChild("sfx"):playcommand("SFX");
						else
							--it's anything else then
							local c = self:GetChild(v)
							if c then
								c:visible(true);
								c:GetChild("sfx"):playcommand("SFX");
							end
						end
					end
				end
				--yeah whatever...
				self:GetChild("revg"):visible(getenv("GradeReverse"..p));
				self:GetChild("revg"):GetChild("sfx"):playcommand("SFX");
			end
			
			oldmods = playermods
		end;
		--speed
		ModiconRow("1x")..{ Name="%dx" };
		--reserved for noteskins
		Def.ActorFrame {
			Name="Noteskin";
			ModiconRow("")..{ Name="skinframe"; };
			Def.Sprite { Name="skingraphic"; };
		};
		--the rest
		ModiconRow("RS")..{ Name="SuperShuffle" };
		ModiconRow("MR")..{ Name="Left" };
		ModiconRow("VN")..{ Name="Hidden" };
		ModiconRow("FD")..{ Name="Dark" };
		ModiconRow("XM")..{ Name="XMode" };
		ModiconRow("RG")..{ Name="revg" };
	}
end

local function DifficultyMeter(p)
	return LoadFont("_impact 50px")..{
		Text="";
		SetCommand=function(self)
			if not GAMESTATE:IsHumanPlayer(p) then return end;
			
			local steps = GAMESTATE:GetCurrentSteps(p)
			local diff = steps:GetDifficulty()
			local stype = steps:GetStepsType()
			local desc = steps:GetDescription()
			local meter = steps:GetMeter()
			--local autogen = steps:IsAutogen() and "AUTO " or ""
			
			--compatible
			local text = DifficultyAndStepstypeToString(diff,stype,desc)
			text = string.upper(text);
			
			self:settextf("%s [%d]",text,meter);
		end;
		CurrentSongChangedMessageCommand=function(self)
			local song = GAMESTATE:GetCurrentSong()
			if not song then self:visible(false); return end
			self:visible(true);
		end;
	};
end

local function SongInformationDisplay()
	local index = 1
	return LoadFont("_arial black 20px")..{
		--Text="asd";
		SetCommand=function(self)
			local song = GAMESTATE:GetCurrentSong()
			if not song then self:diffusealpha(0); return end
			
			local title = song:GetDisplayFullTitle();
			local artist = song:GetDisplayArtist();
			local bpm = song:GetDisplayBpms() --GetTimmingData():GetActualBPM()
			local group = song:GetGroupName();
			local length = song:MusicLengthSeconds();
			if length == 150 then
				length = "Lengthless"
			else
				local longmara
				if song:IsLong() then
					longmara = " Long song"
				elseif song:IsMarathon() then
					longmara = " Marathon song"
				else
					longmara = ""
				end
				length = string.format("%02d:%02d%s",length/60,length%60,longmara)
			end
			
			if bpm[1] == bpm[2] then
				bpm = string.format("BPM %d", bpm[1])
			else
				bpm = string.format("BPM %d - %d",bpm[1],bpm[2])
			end
			
			local metadata = {title,artist,bpm,group,length}
			
			local toset = metadata[index]
			
			index = index+1
			if index > #metadata then index = 1 end
			
			--self:finishtweening();
			self:settext(toset);
			self:linear(0.2);
			self:diffusealpha(1);
			self:sleep(1);
			self:linear(0.2)
			self:diffusealpha(0);
			self:queuecommand("Set");
		end;
		CurrentSongChangedMessageCommand=function(self)
			--reset index
			index = 1
			--start looping
			self:stoptweening();
			self:linear(0.2)
			self:diffusealpha(0);
			self:queuecommand("Set")
		end;
		OffCommand=cmd(finishtweening);
	};
end

return Def.ActorFrame {
	DifficultyMeter(PLAYER_1)..{
		InitCommand=cmd(FromCenterX,-120;FromTop,100;zoom,0.75;diffuse,color("#646464");Stroke,color("#ffffff"));
		CurrentStepsP1ChangedMessageCommand=cmd(playcommand,"Set");
	};
	DifficultyMeter(PLAYER_2)..{
		InitCommand=cmd(FromCenterX,120;FromTop,100;zoom,0.75;diffuse,color("#646464");Stroke,color("#ffffff"));
		CurrentStepsP2ChangedMessageCommand=cmd(playcommand,"Set");
	};
	Draw.RoundBox(SCREEN_WIDTH*0.75+10,50,color("#ffffff"))..{
		InitCommand=cmd(CenterX;FromBottom,-75);
	};
	Draw.RoundBox(SCREEN_WIDTH*0.75,40,5,5,color("#0000c8"))..{
		InitCommand=cmd(CenterX;FromBottom,-75);
	};
	SongInformationDisplay()..{
		InitCommand=cmd(CenterX;FromBottom,-75;diffuse,color("#009600");Stroke,color("#000000"));
	};
	ModIcons(PLAYER_1)..{
		InitCommand=cmd(FromLeft,30;FromTop,240);
		Condition=not GetUserPrefB("OptionsMode");
	};
	ModIcons(PLAYER_2)..{
		InitCommand=cmd(FromRight,-30;FromTop,240);
		Condition=not GetUserPrefB("OptionsMode");
	};
}