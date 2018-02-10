local playMode = GAMESTATE:GetPlayMode()
local slideTime = 1.1;
local slideWait = 1.25;
local bottomSlide = 0.76;
local easeTime = 0.10;

local playMode = GAMESTATE:GetPlayMode()
local sStage = ""
sStage = GAMESTATE:GetCurrentStage()

if playMode ~= 'PlayMode_Regular' and playMode ~= 'PlayMode_Rave' and playMode ~= 'PlayMode_Battle' then
  sStage = playMode;
end;


local t = Def.ActorFrame {};
t[#t+1] = Def.Sprite {
	InitCommand=cmd(Center;diffusealpha,1);
	BeginCommand=cmd(LoadFromCurrentSongBackground);
	StartTransitioningCommand=function(self)
		if PREFSMAN:GetPreference("StretchBackgrounds") then
			self:SetSize(SCREEN_WIDTH,SCREEN_HEIGHT)
		else
			self:scale_or_crop_background()
		end
		self:linear(1)
		self:diffusealpha(0)
	end;
};

-- BG for credits
t[#t+1] = Def.ActorFrame {
	OnCommand=cmd(smooth,bottomSlide;addy,240;sleep,0.2;diffusealpha,0;);
	-- Behind stage graphic
	Def.Quad {
		InitCommand=cmd(vertalign,bottom;x,SCREEN_CENTER_X;y,SCREEN_BOTTOM-110;zoomto,SCREEN_WIDTH,120;);
		OnCommand=function(self)
			self:diffuse(color("#000000")):diffusealpha(0.8);
		end
	};
	-- Behind song
	Def.Quad {
		InitCommand=cmd(vertalign,bottom;x,SCREEN_CENTER_X;y,SCREEN_BOTTOM;zoomto,SCREEN_WIDTH,110;);
		OnCommand=function(self)
			self:diffuse(color("#000000")):diffusealpha(0.9);
		end
	};
};


local stage_num_actor= THEME:GetPathG("ScreenStageInformation", "Stage " .. ToEnumShortString(sStage), true)
if stage_num_actor ~= "" and FILEMAN:DoesFileExist(stage_num_actor) then
	stage_num_actor= LoadActor(stage_num_actor)
else
	-- Midiman:  We need a "Stage Next" actor or something for stages after
	-- the 6th. -Kyz
	local curStage = GAMESTATE:GetCurrentStage();
	stage_num_actor= Def.BitmapText{
		Font= "Common Normal",  Text= thified_curstage_index(false) .. " Stage",
		InitCommand= function(self)
			self:zoom(1.5)
			self:strokecolor(Color.Black)
			self:diffuse(StageToColor(curStage));
			self:diffusetopedge(ColorLightTone(StageToColor(curStage)));
		end
	}
end

t[#t+1] = Def.ActorFrame {
	InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y+190);
	OnCommand=cmd(smooth,bottomSlide;addy,240;sleep,0.2;diffusealpha,0;);

	stage_num_actor .. {
		OnCommand=cmd(zoom,1;diffusealpha,1);
	};
};

-- Step author credits
	if GAMESTATE:IsHumanPlayer(PLAYER_1) == true then
	t[#t+1] = Def.ActorFrame {
	InitCommand=cmd(y,SCREEN_BOTTOM-80;x,SCREEN_LEFT+40;);
	OnCommand=cmd(smooth,bottomSlide;addy,240;sleep,0.2;diffusealpha,0;);
		LoadFont("Common Italic Condensed") .. {
		  OnCommand=cmd(playcommand,"Set";horizalign,left;diffuse,color("#FFFFFF"););
          SetCommand=function(self)
			stepsP1 = GAMESTATE:GetCurrentSteps(PLAYER_1)
			local song = GAMESTATE:GetCurrentSong();
			if song then
				if stepsP1:GetAuthorCredit() ~= "" then
					self:settext(string.upper(THEME:GetString("OptionTitles","Step Author")) .. ":");
				else
					self:settext("")
				end
			else
				self:settext("")
			end
         end
		};
		LoadFont("Common Fallback Font") .. {
		  InitCommand=cmd(addy,22);
		  OnCommand=cmd(playcommand,"Set";horizalign,left;zoom,0.75;diffuse,color("#FFFFFF"););
          SetCommand=function(self)
			stepsP1 = GAMESTATE:GetCurrentSteps(PLAYER_1)
			local song = GAMESTATE:GetCurrentSong();
			if song then
				if stepsP1 ~= nil then
					self:settext(stepsP1:GetAuthorCredit())
				else
					self:settext("")
				end
			else
				self:settext("")
			end
         end
		};
	};
	end
	
	if GAMESTATE:IsHumanPlayer(PLAYER_2) == true then
	t[#t+1] = Def.ActorFrame {
	InitCommand=cmd(y,SCREEN_BOTTOM-80;x,SCREEN_RIGHT-40;);
	OnCommand=cmd(smooth,bottomSlide;addy,240;sleep,0.2;diffusealpha,0;);
	LoadFont("Common Italic Condensed") .. {
		  OnCommand=cmd(playcommand,"Set";horizalign,right;diffuse,color("#FFFFFF"););
          SetCommand=function(self)
			stepsP2 = GAMESTATE:GetCurrentSteps(PLAYER_2)
			local song = GAMESTATE:GetCurrentSong();
			if song then
				local diff = stepsP2:GetDifficulty();
				if stepsP2:GetAuthorCredit() ~= "" then
					self:settext(string.upper(THEME:GetString("OptionTitles","Step Author")) .. ":");
				else
					self:settext("")
				end
			else
				self:settext("")
			end
         end
		};

	LoadFont("Common Fallback Font") .. {
		  InitCommand=cmd(addy,22);
		  OnCommand=cmd(playcommand,"Set";horizalign,right;zoom,0.75;diffuse,color("#FFFFFF"););
          SetCommand=function(self)
			stepsP2 = GAMESTATE:GetCurrentSteps(PLAYER_2)
			local song = GAMESTATE:GetCurrentSong();
			if song then
				if stepsP2 ~= nil then
					self:settext(stepsP2:GetAuthorCredit())
				else
					self:settext("")
				end
			else
				self:settext("")
			end
          end
	};
	};
	end

-- Song title and artist
t[#t+1] = Def.ActorFrame {
	InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_BOTTOM-80);
	OnCommand=cmd(smooth,bottomSlide;addy,240;sleep,0.2;diffusealpha,0;);
	LoadFont("Common Fallback Font") .. {
		Text=GAMESTATE:IsCourseMode() and GAMESTATE:GetCurrentCourse():GetDisplayFullTitle() or GAMESTATE:GetCurrentSong():GetDisplayFullTitle();
		InitCommand=cmd(diffuse,color("#FFFFFF");maxwidth,SCREEN_WIDTH*0.6);
		OnCommand=cmd(zoom,1;);
	};
	LoadFont("Common Fallback Font") .. {
		Text=GAMESTATE:IsCourseMode() and ToEnumShortString( GAMESTATE:GetCurrentCourse():GetCourseType() ) or GAMESTATE:GetCurrentSong():GetDisplayArtist();
		InitCommand=cmd(diffuse,color("#FFFFFF");maxwidth,SCREEN_WIDTH*0.6);
		OnCommand=cmd(zoom,0.75;addy,24;);
	};
};


-- Pause menu
	t[#t+1]= LoadActor(THEME:GetPathG("", "pause_menu"))

return t;