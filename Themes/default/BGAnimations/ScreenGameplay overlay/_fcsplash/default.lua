--FullCombo base from moonlight by AJ

local pn = ...;
assert(pn);

local t = Def.ActorFrame{};
local pStats = STATSMAN:GetCurStageStats():GetPlayerStageStats(pn);

local function GetPosition(pn)
	if Center1Player() then 
		return SCREEN_CENTER_X
	else
		return THEME:GetMetric("ScreenGameplay","Player".. ToEnumShortString(pn) .. "MiscX");
	end;
end;

-- Reuse screen filter code for width-based explosion
local style = GAMESTATE:GetCurrentStyle()
local cols = style:ColumnsPerPlayer()
local padding = 8 -- 4px on each side
local arrowWidth = 96 -- until noteskin metrics are implemented...
local filterWidth = (arrowWidth * cols) + padding

-- Ripple 
t[#t+1] = Def.ActorFrame {
	InitCommand=cmd(visible,false);
	OffCommand=function(self)		
			if pStats:FullCombo() then
				self:visible(true);
			end;
		end;	
	Def.Quad {
		InitCommand=function(self)
			self:diffusealpha(0):x( GetPosition(pn) ):y(SCREEN_CENTER_Y):blend('add'):zoomto(filterWidth,1)
		end;	
		OffCommand=function(self)		
			if pStats:FullCombo() then
				if pStats:FullComboOfScore('TapNoteScore_W1') == true then
						self:diffuse(color("#A0DBF1"))
					elseif pStats:FullComboOfScore('TapNoteScore_W2') == true then
						self:diffuse(color("#F1E4A2"))
					elseif pStats:FullComboOfScore('TapNoteScore_W3') == true then
						self:diffuse(color("#ABE39B"))
				end	
				self:diffusealpha(1):decelerate(1.4):zoomto(filterWidth,400):diffusealpha(0):fadetop(1):fadebottom(1);
			end;
		end;	
	};
};

-- Milestone ripple
t[#t+1] = Def.ActorFrame {
	InitCommand=cmd(visible,false);
	OffCommand=function(self)		
			if pStats:FullCombo() then
				self:visible(true);
			end;
		end;	
	LoadActor("_splash") .. {
		InitCommand=function(self)
			self:diffusealpha(0):x( GetPosition(pn) ):y(SCREEN_CENTER_Y):blend('add');
		end;	
		OffCommand=function(self)		
			if pStats:FullCombo() then
				if pStats:FullComboOfScore('TapNoteScore_W1') == true then
						self:diffuse(color("#A0DBF1"))
					elseif pStats:FullComboOfScore('TapNoteScore_W2') == true then
						self:diffuse(color("#F1E4A2"))
					elseif pStats:FullComboOfScore('TapNoteScore_W3') == true then
						self:diffuse(color("#ABE39B"))
				end	
				self:zoom(0.5):diffusealpha(0.5):decelerate(1):zoom(1):diffusealpha(0)
			end;
		end;	
	};
};

--W1
t[#t+1] = LoadActor(THEME:GetPathG("FullCombo", "W1 text")) .. {
	InitCommand=cmd(diffusealpha,0;x,GetPosition(pn);y,SCREEN_CENTER_Y);	
	OffCommand=function(self)
		local fct = STATSMAN:GetCurStageStats():GetPlayerStageStats(pn);
		if fct:FullComboOfScore('TapNoteScore_W1') == true then
				self:glowblink():effectperiod(0.05):effectcolor1(color("1,1,1,0")):effectcolor2(color("1,1,1,0.25"))
				self:zoomy(0.75):zoomx(1.4):diffusealpha(0):decelerate(0.4):zoomx(0.75):diffusealpha(1):sleep(0.75):decelerate(0.5):zoom(1.1):diffusealpha(0)
		elseif fct:FullComboOfScore('TapNoteScore_W2') == true then
			self:visible(false);
		elseif fct:FullComboOfScore('TapNoteScore_W3') == true then
			self:visible(false);
		else
			self:visible(false);
		end;
	end;	
};

--W1
t[#t+1] = LoadActor(THEME:GetPathG("FullCombo", "W2 text")) .. {
	InitCommand=cmd(diffusealpha,0;x,GetPosition(pn);y,SCREEN_CENTER_Y);	
	OffCommand=function(self)
		local fct = STATSMAN:GetCurStageStats():GetPlayerStageStats(pn);
		if fct:FullComboOfScore('TapNoteScore_W1') == true then
			self:visible(false);
		elseif fct:FullComboOfScore('TapNoteScore_W2') == true then
				self:zoomy(0.75):zoomx(1.4):diffusealpha(0):decelerate(0.4):zoomx(0.75):diffusealpha(1):sleep(0.5):decelerate(0.3):zoom(1.1):diffusealpha(0)
		elseif fct:FullComboOfScore('TapNoteScore_W3') == true then
			self:visible(false);
		else
			self:visible(false);
		end;
	end;	
};

--W3
t[#t+1] = LoadActor(THEME:GetPathG("FullCombo", "W3 text")) .. {
	InitCommand=cmd(diffusealpha,0;x,GetPosition(pn);y,SCREEN_CENTER_Y);	
	OffCommand=function(self)
		local fct = STATSMAN:GetCurStageStats():GetPlayerStageStats(pn);
		if fct:FullComboOfScore('TapNoteScore_W1') == true then
			self:visible(false);
		elseif fct:FullComboOfScore('TapNoteScore_W2') == true then
			self:visible(false);
		elseif fct:FullComboOfScore('TapNoteScore_W3') == true then
				self:zoomy(0.75):zoomx(1.4):diffusealpha(0):decelerate(0.4):zoomx(0.75):diffusealpha(1):sleep(0.5):decelerate(0.3):zoom(1.1):diffusealpha(0)
		else
			self:visible(false);
		end;
	end;	
};

-- Sound
t[#t+1] = LoadActor("_bolt") .. {
	OffCommand=function(self)
	local fct = STATSMAN:GetCurStageStats():GetPlayerStageStats(pn);
		if fct:FullCombo() then
			self:play();
		end;
	end;
};	

return t;